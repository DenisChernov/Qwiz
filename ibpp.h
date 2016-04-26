///////////////////////////////////////////////////////////////////////////////
//
//	File    : $Id: ibpp.h,v 1.18 2003/02/03 18:11:13 epocman Exp $
//	Subject : IBPP public header file. This is _the_ file you include in your
//			  application files when developing with IBPP.
//
///////////////////////////////////////////////////////////////////////////////
//
//	The contents of this file are subject to the Mozilla Public License
//	Version 1.0 (the "License"); you may not use this file except in
//	compliance with the License. You may obtain a copy of the License at
//	http://www.mozilla.org/MPL/
//
//	Software distributed under the License is distributed on an "AS IS"
//	basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
//	License for the specific language governing rights and limitations
//	under the License.
//
//	The Original Code is "IBPP 0.9" and all its associated documentation.
//
//	The Initial Developer of the Original Code is T.I.P. Group S.A.
//	Portions created by T.I.P. Group S.A. are
//	Copyright (C) 2000 T.I.P Group S.A.
//	All Rights Reserved.
//
//	Contributor(s) to version 2.0 :
//
//		Olivier Mascia, main coding
//		Matt Hortman, initial linux port
//		Mark Jordan, design contributions
//		Maxim Abrashkin, enhancement patches
//		Many others, excellent suggestions and bug finding
//
///////////////////////////////////////////////////////////////////////////////
//
//	COMMENTS
//	Tabulations should be set every four characters when editing this file.
//
//	When compiling an IBPP project (or IBPP library itself), the following
//	defines should be made on the command-line (or in makefiles) according
//	to the OS platform and compiler used.
//
//	IBPP_WINDOWS or IBPP_LINUX			: to select the platform
//	IBPP_BCC or IBPP_GCC or IBPP_MSVC	: to select the compiler/environment
//
//	See the documentation and makefiles for more information.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __IBPP_H__
#define __IBPP_H__

#ifndef NULL
#define NULL	0
#endif

#if !defined(IBPP_WINDOWS) && !defined(IBPP_LINUX)
#error Please define IBPP_WINDOWS or IBPP_LINUX before compiling !
#endif

#if !defined(IBPP_BCC) && !defined(IBPP_GCC) && !defined(IBPP_MSVC)
#error Please define IBPP_BCC or IBPP_GCC or IBPP_MSVC before compiling !
#endif

#ifdef IBPP_WINDOWS
#include <windows.h>
#else
typedef long long __int64;
#endif

#include <string>
#include <vector>
#include <map>

#if defined(IBPP_BCC) || defined(IBPP_MSVC)
#define HAS_HDRSTOP
#endif

namespace IBPP
{
	//	Typically you use this constant in a call IBPP::CheckVersion as in :
	//	if (! IBPP::CheckVersion(IBPP::Version)) { throw .... ; }
	const unsigned int Version = 0x02000501; // Version == 2.0.5.1

	//	Transaction Access Modes
	typedef enum {amWrite, amRead} TAM;

	//	Transaction Isolation Levels
	typedef enum {ilConcurrency, ilReadDirty,
		ilReadCommitted, ilConsistency} TIL;

	//	Transaction Lock Resolution
	typedef enum {lrWait, lrNoWait} TLR;

	//	Prepared Statement Types
	typedef enum {stUnknown, stUnsupported,
		stSelect, stInsert, stUpdate, stDelete,	stDDL, stExecProcedure,
		stSelectUpdate, stOther} STT;

	//	SQL Data Types
	typedef enum {sdArray, sdBlob, sdDate, sdTime, sdTimestamp, sdString,
				sdSmallint, sdInteger, sdLargeint, sdFloat, sdDouble} SDT;

	//	Array Data Types
	typedef enum {adDate, adTime, adTimestamp, adString,
				adBool, adShort, adInt, adInt64, adFloat, adDouble} ADT;

	// Database Shutdown Modes
	typedef enum {dsForce, dsDenyTrans, dsDenyAttach} DSM;

	//	Interface Wrapper
	template <class T>
	class Ptr
	{
	private:
		T* mObject;

	public:
		void clear(void)		{ if (mObject != NULL) mObject->Release(mObject); }
		T* intf(void) const		{ return mObject; }
		T* operator->() const	{ return mObject; }
		Ptr& operator=(T* p)
		{
			// AddRef _before_ Release gives correct behaviour on self-assigns
			T* tmp = (p == NULL ? NULL : p->AddRef());	// Take care of NULL
			if (mObject != NULL) mObject->Release(mObject);
			mObject = tmp; return *this;
		}
		Ptr& operator=(const Ptr& r)
		{
			// AddRef _before_ Release gives correct behaviour on self-assigns
			T* tmp = (r.intf() == NULL ? NULL : r->AddRef());// Take care of NULL
			if (mObject != NULL) mObject->Release(mObject);
			mObject = tmp; return *this;
		}
		Ptr(T* p)  			{ mObject = p->AddRef(); }
		Ptr(const Ptr& r)	{ mObject = r->AddRef(); }
		Ptr()	   			{ mObject = NULL; }
		~Ptr()	   			{ clear(); }
	};

	//	Some forward declarations to keep the compiler happy
	class IDatabase;
	class ITransaction;
	class IStatement;
	class EventInterface;

	//	Classes Date, Time, Timestamp and DBKey are 'helper' classes.
	//	They help in retrieving or setting some special SQL types.
	//	Dates, times and dbkeys are often read and written as strings in
	//	SQL scripts. When programming with IBPP, we handle those data with
	//	these specific classes, which enhance their usefullness and free us
	//	of format problems (M/D/Y, D/M/Y, Y-M-D ?, and so on...)

	//	Class Date represent purely a Date (no time part specified).
	//	It is usefull in interactions with the SQL DATE type of Interbase.
	//	You can add or substract a number from a Date, that will modify it to
	//	represent the correct date, X days later or sooner. All the Y2K details
	//	taken into account. The assignment operator and conversion operator
	//	that deal with 'int' type are used to assign and extract the date as
	//	a stand alone integer number. This number represents the date as the
	//	number of days elapsed since 31 Dec 1899. That is 1 is 1 Jan 1900,
	//	2 is 2 Jan 1900 and so on... See the IBPP::dtoi and IBPP::itod methods.

	class Date
	{
	private:
		int _Date;		// The date as an integer : 1 == 1 Jan 1900

	public:
		void Clear(void) { _Date = 0; };
		void Today(void);
		void SetDate(int year, int month, int day);
		void GetDate(int& year, int& month, int& day) const;
		void Add(int days);
		void StartOfMonth(void);
		void EndOfMonth(void);
	
		Date() { Clear(); };
		Date(int year, int month, int day);
		Date(const Date&);					// Copy Constructor
		Date(const int&);
		Date& operator=(const Date&);		// Date Assignment operator
		Date& operator=(const int&);		// int Assignment operator
		operator int() const { return _Date; }	// int Conversion operator
		~Date() { };
	};

	//	Class Time represent purely a Time (H:M:S).
	//	It is usefull in interactions with the SQL TIME type of Interbase.
	//	The assignment operator and conversion operator that deal with 'int'
	//	type are used to assign and extract the time as a stand alone integer
	//	number. This number represents the time as the number of seconds
	//	elapsed since midnight. See the IBPP::ttoi and IBPP::itot methods.

	class Time
	{
	private:
		int _Time;		// The time as an integer, seconds since midnight

	public:
		void Clear(void) { _Time = 0; }
		void Now(void);
		void SetTime(int hour, int minute, int second);
		void GetTime(int& hour, int& minute, int& second) const;

		Time() { Clear(); }
		Time(int hour, int minute, int second);
		Time(const Time&);					// Copy Constructor
		Time(const int&);
		Time& operator=(const Time&);		// Time Assignment operator
		Time& operator=(const int&);		// int Assignment operator
		operator int() const { return _Time; }	// int Conversion operator
		~Time() { };
	};

	//	Class Timestamp represent a date AND a time.
	//	It is usefull in interactions with the SQL TIMESTAMP type of Interbase.

	class Timestamp
	{
	private:
		int _Date;		// The date as an integer : 1 == 1 Jan 1900
		int _Time;		// The time as an integer, seconds since midnight

	public:
		void Clear(void) { _Date = 0; _Time = 0; }
		void Today(void);
		void Now(void);

		void SetDate(int year, int month, int day);
		void GetDate(int& year, int& month, int& day) const;
		void SetTime(int hour, int minute, int second);
		void GetTime(int& hour, int& minute, int& second) const;
		void Add(int days);
		void StartOfMonth(void);
		void EndOfMonth(void);

		Timestamp() { Clear(); }
	  	Timestamp(int year, int month, int day);
		Timestamp(int year, int month, int day, int hour, int minute, int second);
		Timestamp(const Timestamp&);			// Copy Constructor
		Timestamp& operator=(const Timestamp&);	// Timestamp Assignment operator
		Timestamp& operator=(const Date&);		// Date Assignment operator
		Timestamp& operator=(const Time&);		// Time Assignment operator
		~Timestamp() { };
	};

	//	Class DBKey cab store a DBKEY, that special value which the hidden
	//	RDB$DBKEY can give you from a select statement. A DBKey is nothing
	//	specific to IBPP. It's a feature of the Firebird database engine.
	//	See its documentation for more information.

	class DBKey
	{
	private:
		void* _DBKey;				// Stores the binary DBKey
		mutable char* _String;		// String (temporary) representation of it
		int _Size;					// Length of DBKey (multiple of 8 bytes) 

		void BlindCopy(const DBKey&);

	public:
		void Clear(void);
		void SetKey(const void*, int size);
		void GetKey(void*, int size) const;
		const char* AsString(void) const;

		DBKey& operator=(const DBKey&);	// Assignment operator
		DBKey(const DBKey&);			// Copy Constructor
		DBKey();
		~DBKey();
	};

	//	IBPP never return any error codes. In any error condition, IBPP
	//	throws an Exception object.

	class Exception
	{
	public:
		virtual const char* Origin(void) const = 0;
		virtual const char* ErrorMessage(void) const = 0;
		virtual int SqlCode(void) const = 0;
		virtual int EngineCode(void) const = 0;

#ifndef _MSC_VER  // MSVC++ bug workaround
	protected:
#endif
	    Exception() { };
		virtual ~Exception() { };
	};

	//	--- Interface Classes ---
	//	Interfaces IBlob, IArray, IService, IDatabase, ITransaction and
	//	IStatement are at the core of IBPP. Though it is possible to
	//	program your applications by using theses interfaces directly
	//	(as was the case with IBPP 1.x), you should refrain from using
	//	them and prefer the new IBPP Objects Blob, Array, ... (without
	//	the I in front). Those new objects are typedef'd right after
	//	each interface class definition as you can read below.
	//	If you program using the Blob (instead of the IBlob interface
	//	itself), you'll never have to care about AddRef/Release and
	//	you'll never have to care about deleting your objects.

	//	IBlob is the interface to the blob capabilities of IBPP.
	//	Blob is the object class you actually use in your programming.
	//	In Firebird, at the row level, a blob is merely a handle to
	//	a blob, stored elsewhere in the database. Blob allows you to
	//	retrieve such a handle and then read from or write to the
	//	blob, much in the same manner than you would do with a file.

	class IBlob
	{
	public:
		virtual IDatabase* Database(void) const = 0;
		virtual ITransaction* Transaction(void) const = 0;
		virtual void Create(void) = 0;
		virtual void Open(void) = 0;
		virtual void Close(void) = 0;
		virtual void Cancel(void) = 0;
		virtual int Read(void*, int size) = 0;
		virtual void Write(void*, int size) = 0;
		virtual void Info(int* Size, int* Largest, int* Segments) = 0;

		virtual IBlob* AddRef(void) = 0;
		virtual void Release(IBlob*&) = 0;

	protected:
		IBlob() { };
		virtual ~IBlob() { };
	};
	typedef Ptr<IBlob> Blob;

	//	IArray is the interface to the array capabilities of IBPP.
	//	Array is the object class you actually use in your programming.
	//	With an Array object, you can create, read and write Interbase Arrays,
	//	as a whole or in slices.

	class IArray
	{
	public:
		virtual IDatabase* Database(void) const = 0;
		virtual ITransaction* Transaction(void) const = 0;
		virtual void Describe(const std::string& table, const std::string& column) = 0;
		virtual void ReadTo(ADT, void* buffer, int elemcount) = 0;
		virtual void WriteFrom(ADT, const void* buffer, int elemcount) = 0;
		virtual SDT ElementType(void) = 0;
		virtual int ElementSize(void) = 0;
		virtual int ElementScale(void) = 0;
		virtual int Dimensions(void) = 0;
		virtual void Bounds(int dim, int* low, int* high) = 0;
		virtual void SetBounds(int dim, int low, int high) = 0;

		virtual IArray* AddRef(void) = 0;
		virtual void Release(IArray*&) = 0;

	protected:
		IArray() { };
		virtual ~IArray() { };
	};
	typedef Ptr<IArray> Array;

	//	IService is the interface to the service capabilities of IBPP.
	//	Service is the object class you actually use in your programming.
	//	With a Service object, you can do some maintenance work of databases
	//	and servers (backup, restore, create/update users, ...)

	class IService
	{
	public:
	    virtual void Connect(void) = 0;
		virtual void Disconnect(void) = 0;

		virtual void AddUser(const std::string& username, const std::string& password,
			const std::string& first, const std::string& middle, const std::string& last) = 0;
		virtual void ModifyUser(const std::string& username, const std::string& password,
			const std::string& first, const std::string& middle, const std::string& last) = 0;
		virtual void RemoveUser(const std::string& username) = 0;

		virtual void SetPageBuffers(const std::string& dbfile, int buffers) = 0;
		virtual void SetSweepInterval(const std::string& dbfile, int sweep) = 0;
		virtual void SetSyncWrite(const std::string& dbfile, bool) = 0;
		virtual void SetReadOnly(const std::string& dbfile, bool) = 0;
		virtual void SetReserveSpace(const std::string& dbfile, bool) = 0;

		virtual void Shutdown(const std::string& dbfile, DSM mode, int sectimeout) = 0;
		virtual void Restart(const std::string& dbfile) = 0;

		virtual void StartBackup(const std::string& dbfile, const std::string& bkfile,
			bool verbose = false) = 0;
		virtual void StartRestore(const std::string& bkfile, const std::string& dbfile,
			int pagesize = 0, bool verbose = false) = 0;

		virtual const char* WaitMsg(void) = 0;	// With reporting (does not block)
		virtual void Wait(void) = 0;			// Without reporting (does block)

		virtual IService* AddRef(void) = 0;
		virtual void Release(IService*&) = 0;

	protected:
	    IService() { };
		virtual ~IService() { };
	};
	typedef Ptr<IService> Service;

	//	IDatabase is the interface to the database connections in IBPP.
	//	Database is the object class you actually use in your programming.
	//	With a Database object, you can create/drop/connect databases.

	class IDatabase
	{
	public:
		virtual const char* ServerName(void) const = 0;
		virtual const char* DatabaseName(void) const = 0;
		virtual const char* Username(void) const = 0;
		virtual const char* UserPassword(void) const = 0;
		virtual const char* RoleName(void) const = 0;
		virtual const char* CharSet(void) const = 0;
		virtual const char* CreateParams(void) const = 0;

		virtual void Info(int* ODS, int* ODSMinor, int* PageSize, bool* Sync) = 0;
		virtual void Statistics(int* Pages, int* Memory, int* Buffers,
			int* Fetches, int* marks, int* reads, int* writes) = 0;
		virtual void Users(std::vector<std::string>& users) = 0;
		virtual int Dialect(void) = 0;

	    virtual void Create(void) = 0;
		virtual void Connect(void) = 0;
		virtual void Inactivate(void) = 0;
		virtual void Disconnect(void) = 0;
	    virtual void Drop(void) = 0;

		virtual void DefineEvent(const std::string&, EventInterface*) = 0;
		virtual void DropEvent(const std::string&) = 0;
		virtual void ClearEvents(void) = 0;
		virtual void DispatchEvents(void) = 0;

		virtual IDatabase* AddRef(void) = 0;
		virtual void Release(IDatabase*&) = 0;

	protected:
	    IDatabase() { };
	    virtual ~IDatabase() { };
	};
	typedef Ptr<IDatabase> Database;

	//	ITransaction is the interface to the transaction connections in IBPP.
	//	Transaction is the object class you actually use in your programming.
	//	A Transaction object can be associated with more than one Database,
	//	allowing for distributed transactions spanning multiple databases,
	//	possibly located on different servers. IBPP is one among the few
	//	programming interfaces to Firebird that allows you to support
	//	distributed transactions.

	class ITransaction
	{
	public:
	    virtual void AttachDatabase(IDatabase* db, TAM am = amWrite,
				TIL il = ilConcurrency, TLR lr = lrWait) = 0;
	    virtual void DetachDatabase(IDatabase* db) = 0;
	    virtual void Start(void) = 0;
	    virtual void Commit(void) = 0;
	    virtual void Rollback(void) = 0;
	    virtual void CommitRetain(void) = 0;
		virtual void RollbackRetain(void) = 0;

		virtual ITransaction* AddRef(void) = 0;
		virtual void Release(ITransaction*&) = 0;

	protected:
		ITransaction() { };
	    virtual ~ITransaction() { };
	};
	typedef Ptr<ITransaction> Transaction;

	// *** 2.0 BETA *** WORK IN PROGRESS *** THOSE CLASSES ARE NOT COMPLETE ***
	// *** DON'T USE Value and ICursor/Cursor for now, even in beta ***

	// Value is one-size-fits-all type for values read from or written to a DB column
	class Value
	{
	private:
		typedef enum {vEmpty, vNull, vBool, vString, vShort, vInt, vLong,
						vInt64, vFloat, vDouble, vTimestamp, vDate, vTime,
						vDBKey, vBlob, vArray} Type;
		Type mType;
		union
		{
			bool			mBool;
			std::string*	mString;
			short			mShort;
			int				mInt;
			long			mLong;
			__int64			mInt64;
			float			mFloat;
			double			mDouble;
			Timestamp*		mTimestamp;
			Date*			mDate;
			Time*			mTime;
			DBKey*			mDBKey;
			IBlob*			mBlob;
			IArray*			mArray;
		};

	public:
		operator bool() const;
		operator std::string() const;
		operator short() const;
		operator int() const;
		operator long() const;
		operator __int64() const;
		operator float() const;
		operator double() const;
		operator Timestamp() const;
		operator Date() const;
		operator Time() const;
		operator DBKey() const;
		operator Blob() const;
		operator Array() const;

		Value& operator=(const bool&);
		Value& operator=(const std::string&);
		Value& operator=(const short&);
		Value& operator=(const int&);
		Value& operator=(const long&);
		Value& operator=(const __int64&);
		Value& operator=(const float&);
		Value& operator=(const double&);
		Value& operator=(const Timestamp&);
		Value& operator=(const Date&);
		Value& operator=(const Time&);
		Value& operator=(const DBKey&);
		Value& operator=(const Blob&);
		Value& operator=(const Array&);
		Value& operator=(const Value&);

		Value(const bool& rv)			{ mBool = rv; mType = vBool; }
		Value(const std::string& rv)	{ mString = new std::string(rv); mType = vString; }
		Value(const short& rv)			{ mShort = rv; mType = vShort; }
		Value(const int& rv)			{ mInt = rv; mType = vInt; }
		Value(const long& rv)			{ mLong = rv; mType = vLong; }
		Value(const __int64& rv)		{ mInt64 = rv; mType = vInt64; }
		Value(const float& rv)			{ mFloat = rv; mType = vFloat; }
		Value(const double& rv)			{ mDouble = rv; mType = vDouble; }
		Value(const Timestamp& rv)		{ mTimestamp = new Timestamp(rv); mType = vTimestamp; }
		Value(const Date& rv)			{ mDate = new Date(rv); mType = vDate; }
		Value(const Time& rv)			{ mTime = new Time(rv); mType = vTime; }
		Value(const DBKey& rv)			{ mDBKey = new DBKey(rv); mType = vDBKey; }
		Value(const Blob& rv)			{ mBlob = rv->AddRef(); mType = vBlob; }
		Value(const Array& rv)			{ mArray = rv->AddRef(); mType = vArray; }
		Value(const Value& rv);			// Too complex for inline, see value.cpp

		Value()							{ mType = vEmpty; }
		~Value();
	};

	//	IStatement is the interface to the statements execution in IBPP.
	//	Statement is the object class you actually use in your programming.
	//	A Statement object is the work horse of IBPP. All your data
	//	manipulation statements will be done through it. It is also used to
	//	access the result set of a query (when the statement is such), one row
	//	at a time and in strict forward direction.

	class IStatement
	{
	public:
		virtual	IDatabase* Database(void) const = 0;
		virtual ITransaction* Transaction(void) const = 0;
		virtual void Prepare(const std::string&) = 0;
		virtual void Execute(void) = 0;
		virtual void Execute(const std::string&) = 0;
		virtual void ExecuteImmediate(const std::string&) = 0;
		virtual void CursorExecute(const std::string& cursor) = 0;
		virtual void CursorExecute(const std::string& cursor, const std::string&) = 0;
		virtual bool Fetch(void) = 0;
		virtual int AffectedRows(void) = 0;
		virtual void Close(void) = 0;
		virtual STT Type(void) = 0;

		virtual void SetNull(int) = 0;
		virtual void Set(int, bool) = 0;
		virtual void Set(int, const char*) = 0;	// DEPRECATED
		virtual void Set(int, const std::string&) = 0;
		virtual void Set(int, short value) = 0;
		virtual void Set(int, int value) = 0;
		virtual void Set(int, long value) = 0;
		virtual void Set(int, __int64 value) = 0;
		virtual void Set(int, float value) = 0;
		virtual void Set(int, double value) = 0;
		virtual void Set(int, const Timestamp& value) = 0;
		virtual void Set(int, const Date& value) = 0;
		virtual void Set(int, const Time& value) = 0;
		virtual void Set(int, const DBKey& value) = 0;
		virtual void Set(int, const Blob& value) = 0;
		virtual void Set(int, const Array& value) = 0;
		virtual void Set(int, const Value&) = 0;

		virtual bool IsNull(int) = 0;
		virtual bool Get(int, bool*) = 0;
		virtual bool Get(int, char*, int = 0) = 0;	// DEPRECATED
		virtual bool Get(int, std::string&) = 0;
		virtual bool Get(int, short*) = 0;
		virtual bool Get(int, int*) = 0;
		virtual bool Get(int, long*) = 0;
		virtual bool Get(int, __int64*) = 0;
		virtual bool Get(int, float*) = 0;
		virtual bool Get(int, double*) = 0;
		virtual bool Get(int, Timestamp& value) = 0;
		virtual bool Get(int, Date& value) = 0;
		virtual bool Get(int, Time& value) = 0;
		virtual bool Get(int, DBKey& value) = 0;
		virtual bool Get(int, Blob& value) = 0;
		virtual bool Get(int, Array& value) = 0;
		virtual const Value Get(int) = 0;

		virtual bool IsNull(const std::string&) = 0;
		virtual bool Get(const std::string&, bool*) = 0;
		virtual bool Get(const std::string&, char*, int = 0) = 0;	// DEPRECATED
		virtual bool Get(const std::string&, std::string&) = 0;
		virtual bool Get(const std::string&, short*) = 0;
		virtual bool Get(const std::string&, int*) = 0;
		virtual bool Get(const std::string&, long*) = 0;
		virtual bool Get(const std::string&, __int64*) = 0;
		virtual bool Get(const std::string&, float*) = 0;
		virtual bool Get(const std::string&, double*) = 0;
		virtual bool Get(const std::string&, Timestamp& value) = 0;
		virtual bool Get(const std::string&, Date& value) = 0;
		virtual bool Get(const std::string&, Time& value) = 0;
		virtual bool Get(const std::string&, DBKey& value) = 0;
		virtual bool Get(const std::string&, Blob& value) = 0;
		virtual bool Get(const std::string&, Array& value) = 0;
		virtual const Value Get(const std::string&) = 0;

		virtual int ColumnNum(const std::string&) = 0;
		virtual const char* ColumnName(int) = 0;
		virtual const char* ColumnAlias(int) = 0;
		virtual const char* ColumnTable(int) = 0;
		virtual SDT ColumnType(int) = 0;
		virtual int ColumnSize(int) = 0;
		virtual int ColumnScale(int) = 0;

		virtual int Columns(void) = 0;

		virtual IStatement* AddRef(void) = 0;
		virtual void Release(IStatement*&) = 0;

	protected:
		IStatement() { };
	    virtual ~IStatement() { };
	};
	typedef Ptr<IStatement> Statement;

	// *** 2.0 BETA *** WORK IN PROGRESS *** THOSE CLASSES ARE NOT COMPLETE ***
	// *** DON'T USE Value and ICursor/Cursor for now, even in beta ***

	//	ICursor is a kind of specialized IStatement, used solely for SELECT
	//	statements. It is especially usefull for positional updates of the rows
	//	returned by a SELECT. In other words, any situation where you SELECT
	//	a range of rows, want to browse them and update them. ICursor buffers
	//	the rows. It can use a lot of memory, but only do so when required.
	//	It can't delete or insert rows, though. Just read them and update them.
	//	After a successfull Execute, the Cursor is a special state where NO
	//	row has yet been fetched. It is just as if the current position in the
	//	cursor is set *before* the first row. A call to Next() when in that
	//	state actually fetches the first row, as First() would do.
	//	This allows you to make a loop of calls to Next() to fetch all the
	//	rows in the same way as Statement::Fetch() does.
	//	Prev() moves to the previous row.
	//	Last() moves to the last one (actually fetching and buffering all rows).
	//	Count() counts the rows (actually fetching and buffering all rows).
	//	Seek() positions on the row n (first is 0, last is Count()-1).
	//	Update() posts all the updates made on each updated row to the database.
	//	If PostUpdates() is not called and the Cursor is closed, updates are
	//	discarded. The same happens if CancelUdpates() is called.
	//
	//	Columns of the current row are accessed both for read and for write
	//	through the array operator [], using either a column number or a
	//	string with the column name.
	//
	//	Well... this is a first shot at the design of this beast.
	//	Comments more than welcome !

	class ICursor
	{
	public:
		virtual	IDatabase* Database(void) const = 0;
		virtual ITransaction* Transaction(void) const = 0;
		virtual void Prepare(const std::string&) = 0;
		virtual void Execute(void) = 0;
		virtual void Execute(const std::string&) = 0;
		virtual void First(void) = 0;
		virtual bool Next(void) = 0;
		virtual bool Prev(void) = 0;
		virtual void Last(void) = 0;
		virtual int Count(void) = 0;
		virtual void Seek(int) = 0;
		virtual void PostUpdates(void) = 0;
		virtual void CancelUpdates(void) = 0;
		virtual void Close(void) = 0;

		virtual void SetNull(int) = 0;
		virtual void Set(int, const Value&) = 0;

		virtual bool IsNull(int) = 0;
		virtual const Value& operator[] (int) const = 0;
		virtual Value& operator[] (int) = 0;

		virtual bool IsNull(const std::string&) = 0;
		virtual const Value& operator[] (const std::string&) const = 0;
		virtual Value& operator[] (const std::string&) = 0;

		virtual int ColumnNum(const std::string&) = 0;
		virtual const char* ColumnName(int) = 0;
		virtual const char* ColumnAlias(int) = 0;
		virtual const char* ColumnTable(int) = 0;
		virtual SDT ColumnType(int) = 0;
		virtual int ColumnSize(int) = 0;
		virtual int ColumnScale(int) = 0;
		virtual int Columns(void) = 0;

		virtual ICursor* AddRef(void) = 0;
		virtual void Release(ICursor*&) = 0;

	protected:
		ICursor() { };
	    virtual ~ICursor() { };
	};
	typedef Ptr<ICursor> Cursor;

	//	--- Factories ---
	//	These methods are the only way to get one of the above Interfaces.
	//	They are at the heart of how you program using IBPP.
	//	For instance, to get access to a database, you'll write code
	//	similar to this :
	//	{
	//		Database db = DatabaseFactory("server", "databasename",
	//						"user", "password");
	//		db->Connect();
	//		...
	//		db->Disconnect();
	//	}

	IService* ServiceFactory(const std::string& ServerName,
		const std::string& UserName, const std::string& UserPassword);

	IDatabase* DatabaseFactory(const std::string& ServerName,
		const std::string& DatabaseName, const std::string& UserName,
			const std::string& UserPassword, const std::string& RoleName,
				const std::string& CharSet, const std::string& CreateParams);

	inline IDatabase* DatabaseFactory(const std::string& ServerName,
		const std::string& DatabaseName, const std::string& UserName,
			const std::string& UserPassword)
		{ return DatabaseFactory(ServerName, DatabaseName, UserName, UserPassword, "", "", "");	}

	ITransaction* TransactionFactory(IDatabase* db,
		TAM am = amWrite, TIL il = ilConcurrency, TLR lr = lrWait);

	inline ITransaction* TransactionFactory(Database& db,
		TAM am = amWrite, TIL il = ilConcurrency, TLR lr = lrWait)
			{ return TransactionFactory(db.intf(), am, il, lr); }

	IStatement* StatementFactory(IDatabase* db, ITransaction* tr,
		const std::string& sql);

	inline IStatement* StatementFactory(IDatabase* db, ITransaction* tr)
		{ return StatementFactory(db, tr, ""); }

	inline IStatement* StatementFactory(Database& db, Transaction& tr,
		const std::string& sql)
			{ return StatementFactory(db.intf(), tr.intf(), sql); }

	inline IStatement* StatementFactory(Database& db, Transaction& tr)
			{ return StatementFactory(db.intf(), tr.intf(), ""); }

	ICursor* CursorFactory(IDatabase* db, ITransaction* tr);
	inline ICursor* CursorFactory(Database& db, Transaction& tr)
			{ return CursorFactory(db.intf(), tr.intf()); }

	IBlob* BlobFactory(IDatabase* db, ITransaction* tr);
	inline IBlob* BlobFactory(Database& db, Transaction& tr)
		{ return BlobFactory(db.intf(), tr.intf()); }

	IArray* ArrayFactory(IDatabase* db, ITransaction* tr);
	inline IArray* ArrayFactory(Database& db, Transaction& tr)
		{ return ArrayFactory(db.intf(), tr.intf()); }

	//	Class EventInterface is merely a pure interface.
	//	It is _not_ implemented by IBPP. It is just base class definition
	//	from which your own event interface classes have to derive from.
	//
	//	The Event handling system from the class Database requires the
	//	programmer to give an object pointer when defining a new event.
	//	When that event will fire, IBPP will call the method ibppEventHandler
	//	on the object registered with the event.
	//	So in order to catch events, the programmer must derive a class from
	//	EventInterface, implement 'ibppEventHandler' to do anything wanted
	//	when the event triggers and register an instance of that derived class
	//	when defining the event trap.

	class EventInterface
	{
	public:
		virtual void ibppEventHandler(IDatabase*, const std::string&, int) = 0;

	protected:
		EventInterface() { };
		virtual ~EventInterface() { };
	};

	//	IBPP uses a self initialization system. Each time an object that may
	//	require the usage of the Interbase client C-API library is used, the
	//	library internal handling details are automatically initialized, if not
	//	already done. You can kick this initialization at the start of an
	//	application by calling IBPP::CheckVersion(). This is recommended,
	//	because IBPP::CheckVersion will assure you that YOUR code has been
	//	compiled against a compatible version of the library.

	bool CheckVersion(unsigned long);
	int GDSVersion(void);

	//	Finally, here are some date and time conversion routines used by IBPP
	//	and that may be helpful at the application level. They do not depend
	//	on anything related to Firebird/Interbase. Just a bonus.

	void dtoi (int date, int *y, int *m, int *d);
	void itod (int* pdate, int year, int month, int day);
	void ttoi (int itime, int *hour, int *minute, int *second);
	void itot (int* ptime, int hour, int minute, int second);

};

#endif

//
//	EOF
//
