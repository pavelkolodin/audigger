/*
 *  pavelkolodin@gmail.com
 */

#include "CommonGui.h"
#include <fir/str/str.h>

void QStringToCharUTF8(const QString& _qstring, char *_dst)
{
	std::vector<wchar_t> filename_w_tmp;
	filename_w_tmp.resize( _qstring.size() * 2 );
	_qstring.toWCharArray( &filename_w_tmp[0] );

#ifdef WIN32
	// TODO: cp1251
	fir::str::conv_unicode_utf8( &filename_w_tmp[0], _dst );
#else
	fir::str::conv_unicode_utf8( &filename_w_tmp[0], _dst );
#endif

}



bool dialogSaveAsFile(QWidget *_parent, std::vector<char> &_filename_utf8)
{
	QFileDialog dialog(_parent);
	dialog.setViewMode( QFileDialog::Detail );
	dialog.setAcceptMode( QFileDialog::AcceptSave );
	dialog.setFileMode( QFileDialog::AnyFile );
	if ( QDialog::Rejected == dialog.exec() )
		return false;

	QStringList sl = dialog.selectedFiles();
	if ( ! sl.size() )
		return false;

	_filename_utf8.resize( sl[0].size() * 2 );
	QStringToCharUTF8( sl[0], &_filename_utf8[0] );
	return true;
}


bool dialogSaveAsDir(QWidget *_parent, std::vector<char> &_filename_utf8)
{
	QFileDialog dialog(_parent);
	dialog.setViewMode( QFileDialog::Detail );
	dialog.setOption( QFileDialog::ShowDirsOnly );
	dialog.setAcceptMode( QFileDialog::AcceptSave );
	dialog.setFileMode( QFileDialog::AnyFile );  // TODO QFileDialog::DirectoryOnly
	if ( QDialog::Rejected == dialog.exec() )
		return false;

	QStringList sl = dialog.selectedFiles();
	if ( ! sl.size() )
		return false;

	_filename_utf8.resize( sl[0].size() * 2 );
	QStringToCharUTF8( sl[0], &_filename_utf8[0] );
	return true;
}



bool dialogOpenDir(QWidget *_parent, std::vector<char> &_filename_utf8)
{
	QFileDialog dialog(_parent);
	dialog.setViewMode( QFileDialog::Detail );
	dialog.setOption( QFileDialog::ShowDirsOnly );
	dialog.setAcceptMode( QFileDialog::AcceptOpen );
	dialog.setFileMode( QFileDialog::DirectoryOnly );

	if ( QDialog::Rejected == dialog.exec() )
		return false;

	QStringList sl = dialog.selectedFiles();
	if ( ! sl.size() )
		return false;

	_filename_utf8.resize( sl[0].size() * 2 );
	QStringToCharUTF8( sl[0], &_filename_utf8[0] );
	return true;
}


bool dialogOpenFile(QWidget *_parent, const char *_root_path, const char *_specification, std::vector<char> &_result_filename_utf8)
{
	QString path = QFileDialog::getOpenFileName (_parent, /*tr(*/"Open file"/*)*/,	_root_path, /*tr(*/_specification/*)*/ );

	if ( 0 == path.size() )
		return false;

	_result_filename_utf8.resize( path.size() * 2 );
	QStringToCharUTF8( path, &_result_filename_utf8[0] );
	return true;
}
