/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*-----------------------------------------------------------------*/
#include <vcl\vcl.h>
#include <math.h>
#pragma hdrstop
/*----------------------------------------------------------------*/
#include "colors.h"
/*----------------------------------------------------------------*/
float RGBMAX (float r, float g, float b)
{
    float max = r;
    if (g>max) max = g;
    if (b>max) max = b;
    return max;
}
/*----------------------------------------------------------------*/
float RGBMIN (float r, float g, float b)
{
    float min = r;
    if (g<min) min = g;
    if (b<min) min = b;
    return min;
}
/*----------------------------------------------------------------*/
void RGB2HSV (float r, float g, float b, float& h, float& s, float& v)
{
    // r, g, b in [0,1]
    // h in [0,360], s und v in [0,1]
    // ausser wenn s=0, dann h=UNDEFINED>360

    float max, min, delta;

    max = RGBMAX (r, g, b);
    min = RGBMIN (r, g, b);

    v = max;

    if (max!=0.0)
        s = (max-min)/max;
    else
        s = 0.0;
    if (s==0.0) {
        h = UNDEFINED;
        return;
    }

    delta = max-min;
    if (r==max)
        h = (g-b)/delta;
    else if (g==max)
        h = 2.0+(b-r)/delta;
    else if (b==max)
        h = 4.0+(r-g)/delta;
    h *= 60.0;
    if (h<0.0)
        h += 360.0;
}
/*----------------------------------------------------------------*/
void HSV2RGB (float h, float s, float v, float& r, float& g, float&b)
{
    // h in [0,360] oder UNDEFINED, sowie s, v in [0,1]
    // r, g, b in [0,1]

    float f, p, q, t;
    int i;

    if (s==0.0) {
//        if (h!=UNDEFINED) {
//            r = b = g = v;
//            return;
//        }
        r = b = g = v;
        return;
    }

    if (h==360.0)
        h = 0.0;
    h /= 60.0;
    i = (int)floor(h);
    f = h-i;
    p = v*(1-s);
    q = v*(1-s*f);
    t = v*(1-s*(1-f));

    switch (i) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;

        case 1:
            r = q;
            g = v;
            b = p;
            break;

        case 2:
            r = p;
            g = v;
            b = t;
            break;

        case 3:
            r = p;
            g = q;
            b = v;
            break;

        case 4:
            r = t;
            g = p;
            b = v;
            break;

        case 5:
            r = v;
            g = p;
            b = q;
            break;
    }
}
/*----------------------------------------------------------------*/
void RGB2HSV (int _r, int _g, int _b, float& _h, float& _s, float& _v)
{
    float r = ((float)_r)/255.0;
    float g = ((float)_g)/255.0;
    float b = ((float)_b)/255.0;

    RGB2HSV (r, g, b, _h, _s, _v);	
}
/*----------------------------------------------------------------*/
void HSV2RGB (float _h, float _s, float _v, int& _r, int& _g, int& _b)
{
    float r, g, b;
    HSV2RGB (_h, _s, _v, r, g, b);

    _r = (int)(r*255.0);
    _g = (int)(g*255.0);
    _b = (int)(b*255.0);
}
/*----------------------------------------------------------------*/
