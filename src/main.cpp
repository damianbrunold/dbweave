/*  DB-WEAVE textile CAD/CAM software - http://www.brunoldsoftware.ch
    Copyright (C) 1998  Damian Brunold
    Copyright (C) 2009  Damian Brunold
    Copyright (C) 2026  Damian Brunold (Qt 6 port)

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

#include <QApplication>
#include <QMainWindow>

int main (int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("Brunold Software");
    QApplication::setOrganizationDomain("brunoldsoftware.ch");
    QApplication::setApplicationName("DB-WEAVE");
    QApplication::setApplicationVersion("0.1.0");

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("DB-WEAVE"));
    window.resize(1024, 768);
    window.show();

    return app.exec();
}
