/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*  FileProperties — document metadata (author/organization/remarks).
    Verbatim port of legacy/properties.cpp. Newlines in `remarks` are
    encoded as '\\' on disk to survive the line-oriented fileformat
    reader (see RemoveNL / InsertNL).                                 */

#ifndef DBWEAVE_UI_PROPERTIES_H
#define DBWEAVE_UI_PROPERTIES_H


class FfWriter;
class FfReader;

struct FileProperties {
private:
    char* author;
    char* organization;
    char* remarks;

    void RemoveNL(char* _data);
    void InsertNL(char* _data);

public:
    FileProperties();
    virtual ~FileProperties();

    void SetAuthor(const char* _author);
    void SetOrganization(const char* _organization);
    void SetRemarks(const char* _remarks);

    const char* Author() const;
    const char* Organization() const;
    const char* Remarks() const;

    void Save(FfWriter* _writer);
    void Load(FfReader* _reader);
    void Init();
};

#endif
