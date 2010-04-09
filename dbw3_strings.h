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
#ifndef dbw3_stringsH
#define dbw3_stringsH
/*-----------------------------------------------------------------*/
#include "language.h"
/*-----------------------------------------------------------------*/
#define NOCENTRALSYMMFOUND LANG_STR("This pattern has no central symmetry", "Das Muster hat keine Zentralsymmetrie")
#define APP_TITLE_WEBEN LANG_STR(" - Loom control - ", " - Webstuhlsteuerung - ")
#define DATEI_UNBENANNT LANG_STR("unnamed", "unbenannt")
#define APP_TITLE "DB-WEAVE"
#define APP_TITLE_DEMO "DB-WEAVE Demo"
#define SERIALNR_DEMO "Demoversion"
#define SAVE_CHANGES LANG_STR("Do you want to save the changes?", "Möchten Sie die Änderungen speichern?").c_str()
#define CAN_OVERWRITE LANG_STR("The file '%s' already exists. Do you want to overwrite it?", "Die Datei '%s' existiert schon. Möchten Sie sie überschreiben?").c_str()
#define CANNOTSAVE LANG_STR("The file could not be saved", "Die Datei konnte nicht gespeichert werden").c_str()
#define CANNOTLOAD1 LANG_STR("The file ", "Die Datei ").c_str()
#define CANNOTLOAD2 LANG_STR(" could not be correctly loaded.", " konnte nicht korrekt geladen werden.").c_str()
#define PR_PRINTING1 LANG_STR("Page ", "Seite ").c_str()
#define PR_PRINTING2 LANG_STR(" is being printed.", " wird gedruckt.").c_str()
#define STRG_DATASENT LANG_STR("The data was transmitted", "Die Daten wurden übetragen").c_str()
#define STRG_MAXLIMIT LANG_STR("Starting from position %d you can only save %s more picks. Do you want to continue?", "Ab Position %d sind nur noch %d Schüsse frei. Möchten Sie weiterfahren?").c_str()
#define STRG_WEBPOS LANG_STR("Current weaving position is pick %d", "Aktuelle Webposition ist Schuss %d").c_str()
#define COLINFO_PAL LANG_STR("%lu from %lu", "%lu von %lu").c_str()
#define COLINFO_NOPAL "%lu"
#define GOTO_TRITTFOLGE LANG_STR("Tre&adling", "&Trittfolge").c_str()
#define GOTO_SCHLAGPATRONE LANG_STR("P&egplan", "&Schlagpatrone").c_str()
#define HOMEPAGE "http://www.brunoldsoftware.ch"
#define VERSION "Version "
#define NOVALIDFILE LANG_STR("No valid DB-WEAVE file", "Keine gültige DB-WEAVE Datei").c_str()
#define RECALCRAPPORT LANG_STR("Re&calc repeat", "Rapport neubere&chnen").c_str()
#define SETRAPPORTTOSELECTION LANG_STR("&Set size to selection", "Rapport auf &Selektion setzen").c_str()
#define COULD_NOT_EXPORT LANG_STR("The pattern could not be exported", "Das Muster konnte nicht exportiert werden").c_str()
#define INVALID_KETTRANGE LANG_STR("The warp range is invalid", "Der Kettbereich ist ungültig").c_str()
#define INVALID_SCHUSSRANGE LANG_STR("The weft range is invalid", "Der Schussbereich ist ungültig").c_str()
#define INVALID_SCHAFTRANGE LANG_STR("The shaft range is invalid", "Der Schaftbereich ist ungültig").c_str()
#define INVALID_TRITTRANGE LANG_STR("The treadle range is invalid", "Der Trittbereich ist ungültig").c_str()
#define CURSORMOVE LANG_STR("Cursor movement ", "Cursorbewegung").c_str()
#define CURSORLEFT LANG_STR("left ", "links ")
#define CURSORRIGHT LANG_STR("right ", "rechts ")
#define CURSORUP LANG_STR("up", "oben")
#define CURSORDOWN LANG_STR("down", "unten")
#define CURSORNOMOVE LANG_STR("No cursor movement", "Keine Cursorbewegung")
#define HLINEVERT LANG_STR("Vertical ", "Vertikale ")
#define HLINEHORZ LANG_STR("Horizontal ", "Horizontale ")
#define HLINEPOS LANG_STR("support line, Position ", "Hilfslinie, Position ")
#define ENTWURFSINFO LANG_STR("- Pattern information", "- Entwurfsinformationen")
#define RANGE_GRUNDBINDUNG LANG_STR("Base pattern", "Muster für den Grund")
#define RANGE_BINDUNG LANG_STR("Pattern ", "Muster ")
#define SUBST_RANGE LANG_STR("Range substitution", "Bereichsmusterung")
#define SUBST_BLOCK LANG_STR("Block substitution", "Blockmusterung")
#define MAXCOLORSTEPS LANG_STR("The number of steps has to be in the range 1-150", "Die Anzahl Abstufungen muss im Bereich 1-150 sein")
#define PALETTEIDXRANGE LANG_STR("The palette index has to be in the range 1-236", "Der Palettenindex muss im Bereich 1-236 liegen")

#define WEB_SCHUSSFADEN LANG_STR("Pick ", "Schuss ").c_str()
#define WEB_KLAMMER LANG_STR("Brace ", "Klammer ").c_str()
#define WEB_KLAMMER1 LANG_STR("  Brace ", "  Klammer ").c_str()
#define WEB_REPEAT LANG_STR("  Repetition ", "  Wiederholung ").c_str()
#define WEB_KL_LENGTH LANG_STR("  Length ", "  Länge ").c_str()
#define WEB_KL_START LANG_STR("  Beginn ", "  Anfang ").c_str()
#define WEB_KL_STOP LANG_STR("  End ", "  Ende ").c_str()
#define WEB_KL_REPEAT LANG_STR("  Repetitions ", "  Wiederholungen ").c_str()
#define WEB_CANNOTSEND LANG_STR("The connection to the loom could not be established.\r\nIs the loom powered on? Is he correctly set up?", "Die Verbindung zum Webstuhl konnte nicht aufgebaut werden.\r\nIst der Webstuhl eingeschaltet? Ist er korrekt am Computer angeschlossen?").c_str()
#define WEB_NOKLAMMERN LANG_STR("Since there are no braces set, nothing can be woven.", "Da keine Klammern gesetzt sind, kann nichts gewoben werden.").c_str()
#define WEB_INVALIDPOS LANG_STR("The current position is not a valid weaving position.\r\nDo you want to start with the first valid position?", "Es ist keine gültige aktuelle Webposition vorhanden.\r\nSoll mit der ersten gültigen Webposition angefangen werden?").c_str()
#define WEB_PREFIXINVALIDPOS LANG_STR("Invalid weaving position: ", "Ungültige Webposition: ").c_str()

#define USERDEFEZ_CONTINUOUS LANG_STR("The userdefined Threading has to be continuous", "Der benutzerdefinierte Einzug muss zusammenhängend sein").c_str()
#define USERDEF_TOOLARGE LANG_STR("The pattern can at most be 50x50", "Das Muster darf maximal 50x50 gross sein.").c_str()
#define USERDEF_FREE LANG_STR("free", "frei")
#define USERDEF_SELMUSTER LANG_STR("Choose pattern", "Zu belegendes Muster wählen")
#define USERDEF_PASTEMUSTER LANG_STR("Choose pattern to paste", "Einzufügendes Muster wählen")
#define USERDEF_SELDELETE LANG_STR("Choose patten to delete", "Zu löschendes Muster wählen")
#define USERDEF_MUSTER LANG_STR("Pattern", "Muster")

#define MUSTER LANG_STR("Pattern: ", "Muster: ")
#define ENTW_ALLGEMEIN LANG_STR("General", "Allgemein")
#define ENTW_FARBEN LANG_STR("Colors", "Farben")
#define ENTW_LITZEN LANG_STR("Heddles", "Litzen")
#define ENTW_FLOTTIERUNGEN LANG_STR("Floats", "Flottierungen")
#define ENTW_MUSTERNAME LANG_STR("Pattern name: ", "Mustername: ")
#define ENTW_BINDUNGSGROESSE LANG_STR("Pattern size: ", "Mustergrösse: ")
#define ENTW_BINDUNGSRAPPORT LANG_STR("Pattern repeat: ", "Musterrapport: ")
#define ENTW_SCHAFTCOUNT LANG_STR("Number of shafts: ", "Anzahl Schäfte: ")
#define ENTW_TRITTCOUNT LANG_STR("Number of treadles: ", "Anzahl Tritte: ")
#define ENTW_SCHUSSSEITIG1 LANG_STR("The pattern is weft sided (", "Das Muster ist schussseitig (")
#define ENTW_SCHUSSSEITIG2 LANG_STR("% sinking binding points)", "% Senkungen)")
#define ENTW_KETTSEITIG1 LANG_STR("The pattern is warp sided (", "Das Muster ist kettseitig (")
#define ENTW_KETTSEITIG2 LANG_STR("% rising binding points)", "% Hebungen)")
#define ENTW_GLEICHSEITIG LANG_STR("The pattern is balanced", "Das Muster ist gleichseitig")
#define ENTW_FARBENCOUNT LANG_STR("Number of colors:", "Anzahl Farben:")
#define ENTW_FARBENGESAMT LANG_STR("Total: ", "Gesamt: ")
#define ENTW_FARBENKETTE LANG_STR("In warp: ", "In Kette: ")
#define ENTW_FARBENSCHUSS LANG_STR("In weft: ", "In Schuss: ")
#define ENTW_KETTFARBEN LANG_STR("Warp colors:", "Kettfarben:")
#define ENTW_FARBE LANG_STR("Color ", "Farbe ")
#define ENTW_KETTFAEDEN LANG_STR(" warp threads (", " Kettfäden (")
#define ENTW_SCHUSSFARBEN LANG_STR("Weft colors:", "Schussfarben:")
#define ENTW_SCHUSSFAEDEN LANG_STR(" weft threads (", " Schussfäden (")
#define ENTW_LITZENCOUNT LANG_STR("Number of heddles: ", "Anzahl Litzen: ")
#define ENTW_LITZENAUFTEILUNG LANG_STR("Distribution on the shafts:", "Aufteilung auf die Schäfte:")
#define ENTW_LITZENSCHAFT LANG_STR("Shaft ", "Schaft ")
#define ENTW_LITZEN2 LANG_STR(" Heddles  (", " Litzen  (")
#define ENTW_SCHUSSFLOTT LANG_STR("Weft floats:", "Schussflottierungen:")
#define ENTW_MAXFLOTT LANG_STR("Largest float: ", "Längste Flottierung: ")
#define ENTW_AVGFLOTT LANG_STR("Average float: ", "Mittlere Flottierung: ")
#define ENTW_FLOTTDIST LANG_STR("Distribution by float length:", "Verteilung nach Flottierungslänge:")
#define ENTW_FLOTT1 LANG_STR("Length ", "Länge ")
#define ENTW_FLOTT2 LANG_STR(" floats  (", " Flottierungen  (")
#define ENTW_KETTFLOTT LANG_STR("Warp floats:", "Kettflottierungen:")

#define PLACEHOLDER_AUTOR "&Autor"
#define PLACEHOLDER_AUTOR1 "(&Autor)"
#define PLACEHOLDER_AUTOR2 "&Autorin"
#define PLACEHOLDER_AUTOR3 "(&Autorin)"
#define PLACEHOLDER_ORG "&Organisation"
#define PLACEHOLDER_ORG1 "(&Organisation)"
#define PLACEHOLDER_MUSTER "&Muster"
#define PLACEHOLDER_MUSTER1 "(&Muster)"
#define PLACEHOLDER_DATEI "&Datei"
#define PLACEHOLDER_DATEI1 "(&Datei)"
#define PLACEHOLDER_SEITE "&Seite"
#define PLACEHOLDER_E_AUTOR "&Author"
#define PLACEHOLDER_E_AUTOR1 "(&Author)"
#define PLACEHOLDER_E_ORG "&Organisation"
#define PLACEHOLDER_E_ORG1 "(&Organisation)"
#define PLACEHOLDER_E_MUSTER "&Pattern"
#define PLACEHOLDER_E_MUSTER1 "(&Pattern)"
#define PLACEHOLDER_E_DATEI "&File"
#define PLACEHOLDER_E_DATEI1 "(&File)"
#define PLACEHOLDER_E_SEITE "&Page"
#define PAGE_HEADER_DEFAULT LANG_STR(" - &Pattern (&Author)", " - &Muster (&Autor)")

/*-----------------------------------------------------------------*/
#endif
