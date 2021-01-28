//if fstream is not included ofstream has errors
//because it is somehow gets confused between widechar
//and char
#include <fstream>
#include <iostream>
#include <iterator>

#include <boost/mem_fn.hpp>
#include <boost/foreach.hpp>
//for serialization
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>



#include <otlorm/otlorm1.h>


using namespace std;
using namespace otlorm1;



/* most magic will happen inside the macros here
this will initialize the reflection system (which is
several static maps per class type
protected with mutex (so that an instance of the same class
will not cause the maps to be reinitialized
*/
struct  tb_row
	:public cactiverow_t,
	private boost::noncopyable {
	//tb_otrq_prov
	DECL_OTL_ROW_HPP(tb_row)
		//
		// for each field we do:
		// define order, cpp field type, OTL field type (which is DB field type)
		// field name (that is both a variable name for C++ and a field name for DB
		// field length (non 0 for varchar
		//
		DECL_OTL_FIELD(1, OTL_BIGINT, otl_var_bigint, otrq_prov_id, 0)
		DECL_OTL_FIELD(2, OTL_BIGINT, otl_var_bigint, clnt_dbid, 0)
		DECL_OTL_FIELD(3, otl_datetime, orm_var_timestamp_tz, mydate, 0)
#if defined (ORM_HAS_DECNUM)
		DECL_OTL_FIELD(4, cmoney_t, orm_var_decimal, amount, 0)
#endif
		//define a field that is unique key
		DECL_OTL_FIELD_UNIQUE_KEY1(otrq_prov_id)
};


/* this does several things but most importantly
 *  declares a static version of the Row type and that
 *  causes all the machinery for reflection to get
 * initialized
 */
DECL_OTL_ROW_CPP(tb_row)
BOOST_CLASS_EXPORT(tb_row)
BOOST_SERIALIZATION_SHARED_PTR(tb_row)


// a collection of rows is activetable which is
// by default it is multiset, but you can pass in 
// tb_row::tThisClassSet  or tb_row::tThisClassVector
// as a second argument to template below
typedef cactivetable_t<tb_row> tTEST_TB3;
otl_connect db; // connect object

// insert rows into table

void insert() {
	//create a table of rows
	tTEST_TB3 tb_tab;
	for (size_t i = 0; i < 10; ++i) {
		tb_row::tThisClassSharedPtr pRow(new tb_row);
		//active record function to assign random values to row
		pRow->assign_randomval();
		tb_tab.insert(pRow);
	}

	/*
	 * show off some operations provided automagically with active row
	 */

	BOOST_FOREACH(tb_row::tThisClassSharedPtr pRow, tb_tab)	{
		/* assign null to all, this MODIFIES
		   the elments of the set so they become unordered !
		*/
		pRow->assign_otlnull();
	}

	//or

	for_each(tb_tab.begin(), tb_tab.end(),
		boost::mem_fn(&cactiverow_t::assign_otlnull)
	);

	BOOST_FOREACH(tb_row::tThisClassSharedPtr pRow, tb_tab) {
		/* assign some random value to every field,
		   another built in function (just for testing)

		   this MODIFIES
		   the elments of the set so they become unordered !
		*/
		pRow->assign_randomval();
	}


	//meaningless, since tb_tab is a multiset and it is already sorted
	// std::stable_sort(tb_tab.begin(), tb_tab.end(), tb_row::tThisClassOpLessThan());

	/* another example, but this time using lambda
	std::stable_sort(tb_tab.begin(), tb_tab.end(),
		[](const tb_row::tThisClassSharedPtr& p1, const tb_row::tThisClassSharedPtr& p2)
		{
			return (*p1) < (*p2);
		}
		);
	 */


	 /*
	  *  Inserter is a class that is used to generate
	  *  strings to represent SQL statements for insert
	  *  it uses the reflection to introspect tb_row
	  */
	cactiverow_inserter_all_t<tb_row> ins3("tb_test3");
	try {
		//standard OTL stream
		otl_stream s3(11 /*bulk insert size */,
			ins3.get_db_str().c_str(), /* insert statement str*/
			db);

		/* now the insert */
		BOOST_FOREACH(tb_row::tThisClassSharedPtr pRow, tb_tab) {
			s3 << *pRow;
		}
		s3.flush();
		db.commit();
	}
	catch (const otl_exception& e) {
		cout << "ERROR " << e.msg << " " << e.var_info << endl;
		db.rollback();
		return;
	}
	return;
}

/* to be fixed -- update example needs work */
void update(const int af1) {

	otl_stream
		o(1, // buffer size
			"UPDATE tb_test3 "
			"   SET f2=:f2<char[31]> "
			" WHERE f1=:f1<int>",
			// UPDATE statement
			db // connect object
		);

	//o<<"Name changed"<<af1;
#if defined (OTL_UNICODE)
	o << L"Name changed" << af1;
#else
	o << "Name changed" << af1;
#endif
	o << otl_null() << af1 + 1; // set f2 to NULL
}


void save_row(tb_row& r, char const* filename) {
	// make an archive
	std::ofstream ofs(filename);
	assert(ofs.good());
	boost::archive::xml_oarchive oa(ofs);
	r.serialize<boost::archive::xml_oarchive>(oa, 0);
	oa << BOOST_SERIALIZATION_NVP(r);
	//serializeFrom__boost
}


void save_table(const tTEST_TB3::tThisClassParent& t, const char* filename) {
	// make an archive
	std::ofstream ofs(filename);
	assert(ofs.good());
	boost::archive::xml_oarchive oa(ofs);
	oa << boost::serialization::make_nvp("MYTB", t);
}


void load_table(tTEST_TB3::tThisClassParent& t,
	const char* filename) {
	// open the archive
	std::ifstream ifs(filename);
	assert(ifs.good());
	boost::archive::xml_iarchive ia(ifs);
	ia >> boost::serialization::make_nvp("MYTB", t);
}




/* simplest use case
Note that this function stays the same no matter
how many fields are in the table (this was the goal --
to make it so in the simple cases fields need to be typed
only once in a declaration)
*/
void selectall(void) {
	tb_row row;
	cactivetable_t<tb_row> tb_tab;

	otl_stream stream;
	//shortcut to select all fields/all rows 
	tb_tab.get_selectall_stream("tb_test3", stream, db);
	//you can add 'where clause criteria here .... '

	//now read into the in memory table
	//this is a shortcut
	tb_tab.read_intothis(stream);

	//print it out (shows that the table and rows
	//are compatible with some STL algorithms
	//in this particular case the rows provide a 'nice print'
	//to ostream class (or wostream)
	copy(tb_tab.begin(), tb_tab.end(),
		ostream_iterator<tb_row::tThisClassSharedPtr>(cout, "\n"));
}


void select(const int af1) {
    // DSL (domain specific language) for criteria.
	// These are really just overloaded operators... (TODO come up with a better DSL)
    //
	// ^ means comma
	// <fieldname>__bn means bind name of a field  (that is ::fldnm 
	// to be use for binding 
	//    
	// <fieldname>__lf means local name of a field (basically a string name)
	//
	//

	cactive_criteria_t tbwhere;
	tbwhere =
		// this would eventually create a string
		/*
		  select otrq_prov_id,clnt_dbid,mydate\:\:varchar,amount\:\:varchar from tb_test3
		  where amount > 111.3333
		*/
		//::<type> is a postgres idiom to do typecasting
		//my ORM needs to do those tricks for all the databases
		//
#if defined (ORM_HAS_DECNUM)
		tb_row::amount__lf() > cmoney_t("111.3333");
#else
		tb_row::clnt_dbid__lf() > 6;
#endif
	//select all columns, and rows that fit the 'where'
	// criteria
	cactiverow_selector_all_t<tb_row> sel("tb_test3", tbwhere);
	//cout<<sel.get_db_str()<<endl;

	//open up the typical otl stream
	otl_stream s(50,
		sel.get_db_str().c_str(),
		db);

	//rewind means execute if nothing to be supplied
	//into the select stream      
	s.rewind();

	tb_row r;
	tTEST_TB3 tb_tab__SelectIntoThis;
	//this is typical how to iterate over OTL stream
	while (!s.eof()) {
		//read the row
		tb_row::tThisClassSharedPtr pRow(new tb_row);
		s >> *pRow;

		/*add it to the in-memory table
		we are using multiseet bydefault
		so multiset has 'insert', if you parametrized the table
		to be vector, the obviously, use push_back
		*/
		tb_tab__SelectIntoThis.insert(pRow);
		//print the whole row out
		cout << *pRow << endl;

		//in this sample, also show how we can access individual field values
		//with accessor methods
		//basically we can just say
		// <fieldname>__valc   or __val
		//valc returns a const and __val returns non const ref
#if defined (ORM_HAS_DECNUM)
		cout << "amount field value is: " << pRow->amount__valc() << endl;
#endif
	}
	cout << "Testing: Select-Archive-Load before save_table size is: " << tb_tab__SelectIntoThis.size() << endl;
	save_table(tb_tab__SelectIntoThis, "aa_ser_tb");

	tb_tab__SelectIntoThis.clear();
	cout << "Testing: Select-Archive-Load cleared in memory table " << endl;
	load_table(tb_tab__SelectIntoThis, "aa_ser_tb");
	cout << "Testing: Select-Archive-Load after load archive into memory table size is: " << tb_tab__SelectIntoThis.size() << endl;
	cout << "Testing: Select-Archive-Load now test out if the data is the same " << endl;

	copy(tb_tab__SelectIntoThis.begin(), tb_tab__SelectIntoThis.end(),
		ostream_iterator<tb_row::tThisClassSharedPtr>(cout, "\n"));
	return;
}




int main() {
	otl_connect::otl_initialize(); // initialize ODBC environment
	try {
		//Make sure you have the ODBC data source below defined in your
		//ODBC settings
		db.rlogon("d/d@ana_prof_aa_aa");
		otl_cursor::direct_exec	(
			db,
			"drop table tb_test3",
			otl_exception::disabled // disable OTL exceptions
		); // drop table

		otl_cursor::direct_exec
		(
			db,
			(cactiverow_t::getscript_createtable<tb_row>("tb_test3")).c_str()

			/* above activrow  function produces something like:

			"
			DROP TABLE IF EXISTS tb_test3 cascade;
			create table tb_test3("
			  "otrq_prov_id BIGINT,"
			  "clnt_dbid BIGINT,"
			  "mydate timestamp,"
			  "amount DECIMAL(19,3)"
			  ")"
			 */
		);  // create table

		insert(); // insert records into the table
		//update(10); // update records in the table
		select(8); // select records from the table
	    //  selectall();
	}

	catch (const otl_exception& p) { // intercept OTL exceptions
		cerr << p.msg << endl; // print out error message
		cerr << p.stm_text << endl; // print out SQL that caused the error
		cerr << p.sqlstate << endl; // print out SQLSTATE message
		cerr << p.var_info << endl; // print out the variable that caused the error
	}

	db.logoff(); // disconnect from the database
	return 0;
}
