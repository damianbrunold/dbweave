/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
*/

/*-----------------------------------------------------------------*/
#ifndef DBWEAVE_DOMAIN_ZENTRALSYMM_H
#define DBWEAVE_DOMAIN_ZENTRALSYMM_H

#include "vcl_compat.h"
/*-----------------------------------------------------------------*/
class ZentralSymmChecker
{
private:
	char* data;
	char* temp;
	int sizex, sizey;
private:
	bool IsSymmetric();
	void RollLeft();
	void RollUp();
public:
 ZentralSymmChecker();
 ZentralSymmChecker (int _sizex, int _sizey);
	virtual ~ZentralSymmChecker();
	void Init (int _sizex, int _sizey);
	void SetData (int _i, int _j, char _val);
	char GetData (int _i, int _j);
	bool IsAlreadySymmetric();
	bool SearchSymmetry();
};
/*-----------------------------------------------------------------*/
#endif
/*-----------------------------------------------------------------*/
