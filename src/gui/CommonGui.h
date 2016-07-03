/*
 *  pavelkolodin@gmail.com
 */

#ifndef _COMMON_GUI_H_
#define _COMMON_GUI_H_

#include <QString>
#include <QFileDialog>
#include <QWidget>
#include <vector>

void QStringToCharUTF8(const QString& _qstring, char *_dst);

// \return false - cancelled
// \return true - OK
bool dialogSaveAsFile(QWidget *_parent, std::vector<char> &_filename_utf8);
bool dialogSaveAsDir(QWidget *_parent, std::vector<char> &_filename_utf8);
bool dialogOpenDir(QWidget *_parent, std::vector<char> &_filename_utf8);
bool dialogOpenFile(QWidget *_parent, const char *_root_path, const char *_specification, std::vector<char> &_result_filename_utf8);

#endif
