/*
 * IceWM
 *
 * Copyright (C) 1997-2002 Marko Macek
 */
#include "config.h"

#ifndef NO_CONFIGURE_MENUS
#include "themes.h"

#include "yapp.h"
#include "ymenu.h"
#include "wmmgr.h"
#include "wmprog.h"
#include "ymenuitem.h"
#include "sysdep.h"
#include "base.h"
#include "prefs.h"
#include "yprefs.h"
#include <dirent.h>
#include "wmapp.h"

#include "intl.h"

extern char *configArg;

void setDefaultTheme(const char *theme) {
    const char *confDir = strJoin(getenv("HOME"), "/.icewm", NULL);
    mkdir(confDir, 0777);
    delete[] confDir;
    const char *themeConfNew = strJoin(getenv("HOME"), "/.icewm/theme.new.tmp", NULL);
    const char *themeConf = strJoin(getenv("HOME"), "/.icewm/theme", NULL);
    int fd = open(themeConfNew, O_RDWR | O_TEXT | O_CREAT | O_TRUNC | O_EXCL, 0666);
#warning "do proper escaping"
    const char *buf = strJoin("Theme=\"", theme, "\"\n", NULL);
    int len = strlen(buf);
    int nlen;
    nlen = write(fd, buf, len);
    delete [] buf;
    close(fd);
    if (nlen == len) {
        rename(themeConfNew, themeConf);
    } else {
        remove(themeConfNew);
    }
    delete[] themeConfNew;
    delete[] themeConf;
}

DTheme::DTheme(const char *label, const char *theme): DObject(label, 0) {
    fTheme = newstr(theme);
}

DTheme::~DTheme() {
    delete[] fTheme;
}

void DTheme::open() {
    if (!fTheme)
        return;

    setDefaultTheme(fTheme);

    const char *bg[] = { ICEWMBGEXE, "-r", 0 };
    int pid = app->runProgram(bg[0], bg);
    app->waitProgram(pid);

    YStringArray args(4);

#if 0
    args.append(app->executable());
    args.append("--restart");
///    args.append("-t");
///    args.append(fTheme);
    
    if (configArg) {
    	args.append("-c");
    	args.append(configArg);
    }

#endif
    wmapp->restartClient(0, 0);
}

ThemesMenu::ThemesMenu(YWindow *parent): ObjectMenu(parent) {
}

void ThemesMenu::updatePopup() {
    refresh();
}

void ThemesMenu::refresh() {
    //msg("theTheme=%s", themeName);
    removeAll();

    char *path;

    path = strJoin(libDir, "/themes/", NULL);
    findThemes(path, this);
    delete path;

    path = strJoin(configDir, "/themes/", NULL);
    findThemes(path, this);
    delete path;

    path = strJoin(YApplication::getPrivConfDir(), "/themes/", NULL);
    findThemes(path, this);
    delete path;

    addSeparator();
    add(newThemeItem(_("Default"), CONFIG_DEFAULT_THEME, CONFIG_DEFAULT_THEME));
}

ThemesMenu::~ThemesMenu() {
}

YMenuItem * ThemesMenu::newThemeItem(char const *label, char const *theme, char const *relThemeName) {
    DTheme *dtheme = new DTheme(label, relThemeName);

    if (dtheme) {
        YMenuItem *item(new DObjectMenuItem(dtheme));

        if (item) {
            //msg("theme=%s", relThemeName);
            item->setChecked(themeName && 0 == strcmp(themeName, relThemeName));
            return item;
        }
    }
    return NULL;
}

void ThemesMenu::findThemes(const char *path, YMenu *container) {
    char const *const tname("/default.theme");
    bool isFirst(true);

    int dplen(strlen(path));
    char *npath = NULL, *dpath = NULL;

    if (dplen == 0 || path[dplen - 1] != '/') {
        npath = strJoin(path, "/", NULL);
        dplen++;
    } else {
        dpath = newstr(path);
    }

    DIR *dir(opendir(path));

    if (dir != NULL) {
        struct dirent *de;
        while ((de = readdir(dir)) != NULL) {
            YMenuItem *im(NULL);
	    npath = strJoin(dpath, de->d_name, tname, NULL);

            if (npath && access(npath, R_OK) == 0) {
		if (isFirst) {
		    isFirst = false;
//		    if (itemCount())
//                        addSeparator();
		    //addLabel(path);
		    //addSeparator();
		}
                char *relThemeName = strJoin(de->d_name, tname, NULL);
                im = newThemeItem(de->d_name, npath, relThemeName);
                {
                    if (im) container->addSorted(im, false);
                }
                delete [] relThemeName;
	    }

            delete [] npath;

	    char *subdir(strJoin(dpath, de->d_name, NULL));
            if (im && subdir) findThemeAlternatives(subdir, de->d_name, im);
            delete [] subdir;
	}

	closedir(dir);
    }

    delete [] dpath;
}

void ThemesMenu::findThemeAlternatives(const char *path, const char *relName,
                                       YMenuItem *item) 
{
    DIR *dir(opendir(path));

    if (dir != NULL) {
        struct dirent *de;
        while ((de = readdir(dir)) != NULL) {
            char const *ext(strstr(de->d_name, ".theme"));

            if (ext != NULL && ext[sizeof("theme")] == '\0' &&
                strcmp(de->d_name, "default.theme"))
            {
                char *npath(strJoin(path, "/", de->d_name, NULL));

                if (npath && access(npath, R_OK) == 0) {
                    YMenu *sub(item->getSubmenu());

                    if (sub == NULL)
                        item->setSubmenu(sub = new YMenu());

                    if (sub) {
                        char *tname(newstr(de->d_name, ext - de->d_name));
                        char *relThemeName = strJoin(relName, "/", 
                                                     de->d_name, NULL);
			sub->add(newThemeItem(tname, npath, relThemeName));
                        delete[] tname;
                        delete[] relThemeName;
                    }
                }
                delete[] npath;
            }
        }
        closedir(dir);
    }
}
#endif
