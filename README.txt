CppORM

CPP implementation of active record / ORM
author:  Vladislav Papayan   vpapayan @ gmai? . com
contributors:  


THIS IS WORK IN PROGRESS, no production system uses this library yet.

It is free, use it anywhere you want -- just please keep names of the author
and contributors in the files. NO WARANTEE of course.



FEATURES:
------------------

*Declare a database table as set of fields ONLY ONCE, and then automatically get
Insert/Delete/Update  operations on tables/individual rows
For example once you declare something like

struct  tb_row
{
  DECL_OTL_FIELD(1,OTL_BIGINT ,otl_var_bigint,otrq_prov_id,0)
  DECL_OTL_FIELD(2, OTL_BIGINT, otl_var_bigint,clnt_dbid,0)  
  DECL_OTL_FIELD(3,otl_datetime,orm_var_timestamp_tz,mydate,0)
  DECL_OTL_FIELD(4,cmoney_t,orm_var_decimal,amount,0)

/* second argument is either c or user defined data type, can be 
int, string, long, double but I have declared csqlstring_t for example
to automagically switch between unicode-16 and regular strings
without changing code 
The data type must satisfy two major properties:
	* it can be streamed into otl_stream
	* it can be streamed into boost Archive types
*/

};

(see the example below for exact syntax)

you will not need to mention the field names again unless you want to access
the data.  That means that if you want to add another field to your table, you
just modify the structure above, and that's it -- you do not have to through
the rest of your code carefully searching for "amount"  strings

If you remove a field from here, and it is used somewhere -- you will immediately
get compile error (because all the fields are type-safe compile time structures and
not just 'strings')

Now
tb_row  my_row  --  is the row

cactivetable_t<tb_row> my_tb; --  is a table  of rows (by default it is std::multiset, but std::vector
can be used too)



*	Database SQL where clauses are modeled with C++ operators 
&& - 'and'
|| - 'or'
^  - 'comma'
etc

*  Supports Serialization!  Finally declare once -- use everywhere in C++
Using boost serialization library.  Therefore you can
read the data from the database serialize it to disk, read it back from disk, and insert
it into the database (see example below) using any of the boost serializers (xml, text, binary).

You do not need to use 'database' to just use the serialization. However all the stuff is
now commingled into one header file -- so database supports will need to be compiled in for now


*  Uses OTL for Database independent database access

*  Thread safe using boost mutex primitives (but very little locking is going on, 
just to check one flag per class type)


*  supports UTF-16 when OTL_UNICODE is defined,  otherwise we are 8 bit ascii clean -- meaning
that reading UTF-8 data works without OTL_UNICODE with regular ascii database drivers


* uses shared pointers (boost::shared_ptr) to store rows in table to avoid memory leaks and prevent
excessive copying.

* for each declared table you can use some convenience functions
	* generate create table string at runtime
 	* assign random values to all the fields for a row
	* assign nulls to all the fields for a row
	* nice-print a row out to an ostream


* supports tricky Database data types right out of the box
	cmoney  -- or more generally NUMBER(19,6) or less for large number arithmetic
	using IBM's decNum library
	
	BIGINT/BIGSERIAL -- using OTL's OTL_BIGING, dates using otl_datetime

	wstring (UTF-16) as well regular strings

*  Most important machinery that I developed is the reflection mechanism for C++
	That's why all the above functionality is possible -- because I can introspect
	a class instance at runtime and find
		all field names, their database field name equivalent, database type,
		cpp type for every field, and many other things.


	I tried to generate as many things at possible at compile time to avoid any
	runtime penalties and to be as 'type-safe as possible.

	However there is a runtime cost associated with introspection.
	I implemented by having 3 static maps for each Class Type (not instance)
	They get initialized as part of static initialization and then used at
	runtime.

	When a new class instance gets created at runtime (say on the heap) - there is
	a mutex lock that gets checked to see if the static maps exist, therefore 
	this is not as fast as having no reflection...



LIMITATIONS:
	Well... this is work in progress.  I decided to publish it now to see ask
	for comments and for help (if there is interest)

	Right now it compiles with VS9, boost 1.38 and latest OTL database access libary
	I am using to test Postgres Database with ODBC driver (both ascii for UTF-8 and unicode)

	Basically any database that is supported by OTL should work, and any platform that is
	supported by Boost.

	But I have not compiled anywhere else (I just recently created the Cmake files (part
	of this distribution) ) so that other platforms can be tested.

	I am pretty sure other DB's will not work right a way for datetime and cmoney (Number (19,6)
	data types -- because certain things are not SQL standard and I had to implement
	typecasts (for example ) for Postgres/ODBC combination.

	There is no 'specific' support for BLOBs -- but OTL suports them so you can use them,
	just do not put them as part of the table declaration (as I have not tested that yet...)

	There are couple of things that will unlikely to be supported:
		and that is Native postgres sql or Mysql drivers -- unless OTL supports them
	See http://otl.sourceforge.net/

	The notion of reflection of inheritance hierarchy is not supported.  In other words you cannot have class B in inherit from class A and then use the ORM on B.	I think partial solutions to this can be implemented, but this is not extremely easy.
	


--------------------------------
HISTORY:

06/20/09	vpapayan 	implemented serialization for activerow and activetable using boost









---------------------------------



Installation:

You will need cmake 2.6 or higher
boost 1.38 or higher
decNumber (see the readme file inside decNumber)
OTL C++ library ( http://otl.sourceforge.net/) (a version is included in this repository, but later one can be used)

run cmake-gui by specifying that the build
to be 'build' and source directory to be 'trunk'

for Windows, use Visual Studio 2008 (that's what I am initialy developing with)

---- Here is an example of how this ORM can be used --- Start reading from the main function ----
-- If you want to quickly look at the code, it is mostly one header file   otlorm1.h  --


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
   :public boost::enable_shared_from_this<tb_row>,
   public cactiverow_t,
   private boost::noncopyable
{
  //tb_otrq_prov
  DECL_OTL_ROW_HPP(tb_row)
    //
    // for each field we do:
    // define order, cpp field type, OTL field type (which is DB field type)
    // field name (that is both a variable name for C++ and a field name for DB
    // field length (non 0 for varchar
    //
  DECL_OTL_FIELD(1,OTL_BIGINT ,otl_var_bigint,otrq_prov_id,0)
  DECL_OTL_FIELD(2, OTL_BIGINT, otl_var_bigint,clnt_dbid,0)  
  DECL_OTL_FIELD(3,otl_datetime,orm_var_timestamp_tz,mydate,0)
  DECL_OTL_FIELD(4,cmoney_t,orm_var_decimal,amount,0)

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
BOOST_SERIALIZATION_SHARED_PTR( tb_row )


// a collection of rows is activetable which is
// by default it is multiset, but you can pass in 
// tb_row::tThisClassSet  or tb_row::tThisClassVector
// as a second argument to template below
typedef cactivetable_t<tb_row> tTEST_TB3;




otl_connect db; // connect object



// insert rows into table

void insert()
{
   
   //create a table of rows
   tTEST_TB3 tb_tab;

    for (size_t i=0; i<10; ++i)
    {
      tb_row::tThisClassSharedPtr pRow(new tb_row);

      //active record function to assign random values to row
      pRow->assign_randomval();      
      tb_tab.insert(pRow);      
    }

    /*
     * show off some operations provided automagically with active row   
     */

    BOOST_FOREACH( tb_row::tThisClassSharedPtr pRow, tb_tab )
    {
          /* assign null to all, this MODIFIES
             the elments of the set so they become unordered !
          */
          pRow->assign_otlnull();
    }

    //or

    for_each(tb_tab.begin(),tb_tab.end(),
        boost::mem_fn(&cactiverow_t::assign_otlnull)
    );



    BOOST_FOREACH( tb_row::tThisClassSharedPtr pRow, tb_tab )
    {
          /* assign some random value to every field,
             another built in function (just for testing)

             this MODIFIES
             the elments of the set so they become unordered !
          */
          pRow->assign_randomval();
    }

    stable_sort(tb_tab.begin(),tb_tab.end());   
   
   /*
    *  Inserter is a class that is used to generate
    *  strings to represent SQL statements for insert
    *  it uses the reflection to introspect tb_row
    */
   cactiverow_inserter_all_t<tb_row> ins3("tb_test3");      
   
   try
   {
   
     //standard OTL stream
      otl_stream s3(11 /*bulk insert size */,
                 ins3.get_db_str().c_str(), /* insert statement str*/
               db);

      /* now the insert */
      BOOST_FOREACH( tb_row::tThisClassSharedPtr pRow, tb_tab )
      {
          s3<<*pRow;
      }

      s3.flush();
      db.commit();
   }
   catch (otl_exception& e)
   {
      cout<<"ERROR "<<e.msg<<" "<<e.var_info<<endl;
      db.rollback();
      return;
   }   
   return;      
}

/* to be fixed -- update example needs work */
void update(const int af1)
// insert rows into table
{
 otl_stream
   o(1, // buffer size
     "UPDATE test_tab "
     "   SET f2=:f2<char[31]> "
     " WHERE f1=:f1<int>",
        // UPDATE statement
     db // connect object
    );

 //o<<"Name changed"<<af1;
 #if defined (OTL_UNICODE)
 o<<L"Name changed"<<af1;
 #else
 o<<"Name changed"<<af1;
 #endif
 
 o<<otl_null()<<af1+1; // set f2 to NULL

}


void save_row( tb_row &r, const char * filename){
    // make an archive
    std::ofstream ofs(filename);
    assert(ofs.good());
    boost::archive::xml_oarchive oa(ofs);

    r.serialize<boost::archive::xml_oarchive>(oa,0);

    oa << BOOST_SERIALIZATION_NVP(r);



    //serializeFrom__boost
}


void save_table(const tTEST_TB3::tThisClassParent &t, const char * filename)
{
    // make an archive
    std::ofstream ofs(filename);
    assert(ofs.good());
    boost::archive::xml_oarchive oa(ofs);
    
    
    oa << boost::serialization::make_nvp("MYTB", t);
}


void load_table (tTEST_TB3::tThisClassParent& t, const char * filename)
{
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
void selectall (void)
{
   tb_row row;
   cactivetable_t<tb_row> tb_tab;      
   
   otl_stream stream;
   //shortcut to select all fields/all rows 
   tb_tab.get_selectall_stream("tb_test3",stream,db);
   //you can add 'where clause criteria here .... '
   
   //now read into the in memory table
   //this is a shortcut
   tb_tab.read_intothis(stream);
   
   //print it out (shows that the table and rows
   //are compatible with some STL algorithms
   //in this particular case the rows provide a 'nice print'
   //to ostream class (or wostream)
   copy (tb_tab.begin(),tb_tab.end(),
   ostream_iterator<tb_row::tThisClassSharedPtr>(cout,"\n"));
}


void select(const int af1)
{
        cactive_criteria_t tbwhere;

       // ^ means comma
       // <fieldname>__bn means bind name of a field  (that is ::fldnm 
       // to be use for binding 
       //    
       // <fieldname>__lf means local name of a field (basically a string name)
       //
       //

      tbwhere=
        // this would eventually create a string
        /*
          select otrq_prov_id,clnt_dbid,mydate\:\:varchar,amount\:\:varchar from tb_test3
          where amount > 111.3333
        */
        //::<type> is a postgres idiom to do typecasting
        //my ORM needs to do those tricks for all the databases
        //
        tb_row::amount__lf() > cmoney_t("111.3333") ;


        //select all columns, and rows that fit the 'where'
        // criteria
        cactiverow_selector_all_t<tb_row> sel("tb_test3",tbwhere);
//cout<<sel.get_db_str()<<endl;

        //open up the typical otl stream
        otl_stream s(50,
              sel.get_db_str().c_str(),
              db);  


      //rewind means execute if nothing to be supplied
      //into the select stream      
      s.rewind();

      tb_row r;
      tTEST_TB3 tb_tab;
      //this is typical how to iterate over OTL stream
      while (!s.eof())
      {
        //read the row
        tb_row::tThisClassSharedPtr pRow(new tb_row);
         s>>*pRow;        

         /*add it to the in-memory table
         we are using multiseet bydefault
         so multiset has 'insert', if you parametrized the table
         to be vector, the obviously, use push_back
         */
         tb_tab.insert(pRow);

         //print the whole row out
         cout<<*pRow<<endl;
         
         //or show how we can access individual field values
         //with accessor methods
         //basically we can just say
         // <fieldname>__valc   or __val
         //valc returns a const and __val returns non const ref

         cout<<"amount field value is: "<<pRow->amount__valc()<<endl;

      }  

      
      cout<<"before save_table size is: "<<tb_tab.size()<<endl;
       save_table(tb_tab, "vladik_ser_tb");

       tb_tab.clear();
       cout<<"cleared table "<<endl;
       load_table(tb_tab, "vladik_ser_tb");
       cout<<"after load table size is: "<<tb_tab.size()<<endl;
       cout<<" now test out if the data is the same "<<endl;



          copy (tb_tab.begin(),tb_tab.end(),
   ostream_iterator<tb_row::tThisClassSharedPtr>(cout,"\n"));


        return;
}




int main()
{

 otl_connect::otl_initialize(); // initialize ODBC environment
 try{
 
  //Make sure you have the ODBC data source below defined in your
   //ODBC settings
  db.rlogon("postgres/postgres@ana_prof_aa_aa"); 

  otl_cursor::direct_exec
   (
    db,
    "drop table tb_test3",
    otl_exception::disabled // disable OTL exceptions
   ); // drop table

  otl_cursor::direct_exec
   (
    db,
    (cactiverow_t::getscript_createtable<tb_row> ("tb_test3")).c_str()

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
//  update(10); // update records in the table
  select(8); // select records from the table
  
  
//  selectall();

 }

 catch(otl_exception& p){ // intercept OTL exceptions
  cerr<<p.msg<<endl; // print out error message
  cerr<<p.stm_text<<endl; // print out SQL that caused the error
  cerr<<p.sqlstate<<endl; // print out SQLSTATE message
  cerr<<p.var_info<<endl; // print out the variable that caused the error
 }

 db.logoff(); // disconnect from the database

 return 0;

}
