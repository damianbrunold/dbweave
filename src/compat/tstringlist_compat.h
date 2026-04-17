/*  DB-WEAVE Qt 6 port - VCL TStringList compatibility (Phase 1)
    Copyright (C) 1998-2026  Damian Brunold
*/

/*  Minimal TStringList-compatible wrapper around QStringList. Only the
    subset of members that legacy DB-WEAVE code actually uses is provided:

      - Add (QString)               -> append
      - Count()                     -> size
      - Strings[i]                  -> operator[]
      - LoadFromFile / SaveToFile   -> QFile-based line I/O (Latin-1 to
                                       match VCL default)
      - Clear

    This is intentionally thin. The dbweave code uses TStringList as a
    plain sequence-of-strings container; nothing in the legacy source
    relies on CommaText, Sorted, Duplicates etc.
*/

#ifndef DBWEAVE_COMPAT_TSTRINGLIST_COMPAT_H
#define DBWEAVE_COMPAT_TSTRINGLIST_COMPAT_H

#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>

class TStringList
{
public:
	TStringList() : Strings(this) {}
	TStringList(const TStringList&) = delete;
	TStringList& operator= (const TStringList&) = delete;

	void Add (const QString& _s)        { items.append(_s); }
	int  Count() const                  { return items.size(); }
	void Clear()                        { items.clear(); }

	QString&       operator[] (int _i)       { return items[_i]; }
	const QString& operator[] (int _i) const { return items[_i]; }

	/*  Legacy callers use list->Strings[i] syntax; expose a proxy so
	    ported code compiles without edits on that access pattern.
	    TStringList is intentionally non-copyable because the proxy
	    stores a back-pointer; VCL code used TStringList via pointer too. */
	struct StringsProxy
	{
		TStringList* owner;
		explicit StringsProxy (TStringList* _o) : owner(_o) {}
		QString&       operator[] (int _i)       { return (*owner)[_i]; }
		const QString& operator[] (int _i) const { return (*owner)[_i]; }
	};
	StringsProxy Strings;

	bool LoadFromFile (const QString& _path)
	{
		QFile f(_path);
		if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
		items.clear();
		QTextStream in(&f);
		while (!in.atEnd())
			items.append(in.readLine());
		return true;
	}

	bool SaveToFile (const QString& _path) const
	{
		QFile f(_path);
		if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;
		QTextStream out(&f);
		for (const QString& line : items)
			out << line << '\n';
		return true;
	}

private:
	QStringList items;
};

#endif /* DBWEAVE_COMPAT_TSTRINGLIST_COMPAT_H */
