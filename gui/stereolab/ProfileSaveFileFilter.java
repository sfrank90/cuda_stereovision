package stereolab;
/* 
** Klasse:      ProfileSaveFileFilter
** Autor:       Christian Werner <cwerner@informatik.hu-berlin.de>
** Version:     1.0 (vom 22. April 2002)
**
** Beschreibung:
**
** Filefilter-Klasse für das Projekt "StereoLab". 
*/

import java.io.File;
import javax.swing.*;
import javax.swing.filechooser.*;

public class ProfileSaveFileFilter extends FileFilter {
    
    // Accept all directories and all raw files.
    public boolean accept(File f) {
        if (f.isDirectory()) {
            return true;
        }

        String extension = getExtension(f);
	if (extension != null) {
            if ((extension.equals("pro")) || (extension.equals("txt"))) {
                    return true;
            } else {
                return false;
            }
    	}

        return false;
    }
    
    // The description of this filter
    public String getDescription() {
        return "StereoLab Profile-Objekte";
    }

    public static String getExtension(File f) {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');

        if (i > 0 &&  i < s.length() - 1) {
            ext = s.substring(i+1).toLowerCase();
        }
        return ext;
    }
}

