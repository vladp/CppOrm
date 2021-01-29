# CppORM
CPP implementation of active record / ORM

**THIS IS WORK IN PROGRESS, no production system uses this library yet.**

It is free, use it anywhere you want -- just please keep names of the author
and contributors in the files. NO WARRANTEE, of course.

## Features and peculiarities at glance:
1. Use Postgres database via ODBC
2. Map a database table into C++ structure
3. Can mix in database fields and transient fields (that are not represented in the database)
4. Has an implementation of BigDecimal / (money) class to work with large amounts without loosing precision
5. Store/Load a select result into a file using boost:archive (XML and other formats are supported)
6. Unicode via wstring
7. DDL such as table create
8. Prepared SQL and Array operations are supported
9. Prettyprint for the data in memory;

## Future aspirations
1. [OTL](http://otl.sourceforge.net/otl3_intro.htm) library supports a number of database, so adding support for those database to this ORM would be great
2. Supporting, simultaneously a relatioshal database and a key-value store, for the use case of near-caches (where portion of the database is replicated in a local key-value store to facilitate faster access and to offload main database)
3. Automated test suit, so that additional work can be quickly verified
4. more compilers and OS combination
5. packaging into vcpkg and other packaging managers
6. leveraging new c++ features to remove the old 'cruft' that was used implement reflection

## Details:


Declare a database table as set of fields ONLY ONCE, and then automatically get
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

(see the example/ex1)

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


*  Supports Serialization! Using boost serialization library.  Therefore you can
read the data from the database serialize it to disk, read it back from disk, and insert
it into the database (see example below) using any of the boost serializers (xml, text, binary).

You do not need to use 'database' to just use the serialization. However, all the stuff is
now commingled into one header file -- so database supports will need to be compiled in for now

* relies on well tested C++ [OTL](http://otl.sourceforge.net/otl3_intro.htm) library (developed and maintained by Sergey Kuchin for over 24 years)


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
	* cmoney  -- or more generally NUMBER(19,6) or less for large number arithmetic
	using IBM's [decNumber](https://github.com/SDL-Hercules-390/decNumber) library
	* BIGINT/BIGSERIAL -- using OTL's OTL_BIGING, dates using otl_datetime
	* wstring (UTF-16) as well regular strings

*  Most important machinery that was developed that enabled all these, is the reflection mechanism for C++
	That's why all the above functionality is possible -- because we can introspect
	a class instance at runtime and find: all field names, their database field name equivalent, database type, cpp type for every field, and many other things.


This library  tries to generate as many things at possible at compile time to avoid any
runtime penalties and to be as 'type-safe as possible.

However there is a runtime cost associated with introspection.
The library implements it by having 3 static maps for each Class Type (not instance)
They get initialized as part of static initialization and then used at runtime.

When a new class instance gets created at runtime (say on the heap) - there is
a mutex lock that gets checked to see if the static maps exist, therefore this is not as fast as having no reflection.



## Limitations:
	Well... this is work in progress.  I decided to publish it now to see ask
	for comments and for help (if there is interest)

	Right now it compiles with Visual Studio 16 2019 toolset v141, boost 1.75 and latest OTL database access libary
	I am using to test Postgres Database with [ODBC driver](https://www.postgresql.org/ftp/odbc/versions/msi/) (both ascii for UTF-8 and unicode)

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
		and that is Native Postgres sql or native Mysql drivers -- unless OTL supports them
	See http://otl.sourceforge.net/

	The notion of reflection of inheritance hierarchy is not supported.  In other words you cannot have class B in inherit from class A and then use the ORM on B.	I think partial solutions to this can be implemented, but this is not extremely easy.
	



## Installation:

* You will need cmake 3.5 or higher
* boost 1.38 or higher
* decNumber is included as a gitsubmodule in decNumber subdirectory, so it will need to be initialized as git submodule:    ` cd trunk/decNumber ` `git submodule init` `git submodule update`
* [OTL C++ library]( http://otl.sourceforge.net/) (an old version is included in this repository, but later ones should work as well)
* Read BUILD.txt on how to use cmake to generate Visual Studio project files
Read example/ex1.cpp for basic usage, start with main function
