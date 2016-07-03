
#ifndef _WidgetToolbar_H_
#define _WidgetToolbar_H_

#include <QtWidgets>
#include <boost/ptr_container/ptr_vector.hpp>
#include "ToolBar.h"
#include "Dragging.h"
#include "Settings.h"

namespace ToolBar
{

class WidgetToolBar : public QWidget
{
	Q_OBJECT
public:
	WidgetToolBar(QWidget *_parent, Settings::Settings &_settings);
	virtual ~WidgetToolBar();
	void clear();

	// default index = -1 - end of layout
	bool insertTool( ToolBar::ToolType _type, int _index = -1);

	// In "edit mode" you cannot press buttons/change regulators,
	// you only see its pictures, drag/drop/remove them by mouse and using WidgetToolBank.
	void setModeEdit( bool _true );

	// assign state to tool
	void setToolState( ToolBar::ToolType _type, unsigned _state );

	// get state of tool
	unsigned getToolState( ToolBar::ToolType _type );

	// Move the state of tool of dial type down. (example: TOOL_TIMESTEP)
	// \return tool state.
	unsigned dialUp( ToolBar::ToolType _type );

	// Move the state of tool of dial type down.
	// \return tool state.
	unsigned dialDown( ToolBar::ToolType _type );

	void enableTool( ToolBar::ToolType _type, bool _enabled );

protected:
	virtual void paintEvent ( QPaintEvent * );
	virtual void resizeEvent ( QResizeEvent * );

	virtual void mousePressEvent ( QMouseEvent * );
	virtual void mouseMoveEvent ( QMouseEvent * );
	virtual void mouseReleaseEvent( QMouseEvent * );

	virtual void dragEnterEvent ( QDragEnterEvent * );
	virtual void dragMoveEvent ( QDragMoveEvent * );
	virtual void dragLeaveEvent ( QDragLeaveEvent * );
	virtual void dropEvent ( QDropEvent * );

	void dragTool(int _index, int _coord);
	void dragToolFinish(int _index);

private:
	// return NULL if not found
	const ToolInfo* findToolInfo__( ToolType );
	const ToolInfo* findToolInfoByName__( const std::string &_name );
	void rebuildLayer__();
	void enableWidgets__();

	// Find widget occupying the place, where the "_coord" hits.
	int whosePlace__( unsigned _coord, int *_offset );

	// Try to invert value of clicked button.
	// This will be done according to tool's policy. For example, if button is not
	// checkable, you cannot set some state to it. Or if the button belongs to some
	// group and is checkable, anoter buttons in the group will be unchecked.
	// (see struct ToolBar::ToolInfo)

	// return: value of setToolState__
	bool toolClicked__( ToolBar::ToolType _type );

	// Recalculate tool's state according to many factors (tool's type, group, flags (see struct ToolBar::ToolInfo)).
	// return:
	// true - state of type changed or this it not checkable tool
	// false - not changed
	// (user must not get signal when checked button pressed again, but must get signal
	// when not checkable button is pressed or unchecked checkable button is pressed).
	bool setToolState__( ToolBar::ToolType _type, unsigned _state );

	// Change visual look of widgets (set buttons down/up, change view of regulators)
	// according to states of states of all tools.
	void applyStatusesToWidgets__( );

	void saveToolsState__();
	void saveLayout__( );

	Settings::Settings &m_settings;

	typedef std::vector < QWidget *> VectorQWidgets;
	//typedef std::vector < ToolBar::ToolType, unsigned > MapStatuses;
	typedef std::vector < unsigned > VectorStates;
	typedef std::vector < const ToolInfo* > VectorToolInfo;
	//typedef std::vector < ToolInfoExtended > VectorToolInfoExtended;

	int m_highlight_coord;
	int m_highlight_index;
	bool m_mode_edit;

	VectorToolInfo m_info;
	VectorToolInfo m_layoutplan;
	VectorQWidgets m_layout_widgets;
	VectorStates m_states;
	VectorStates m_enabled;
	std::vector < QPixmap > m_layout_pic;

	Dragging< WidgetToolBar, int, int > m_drg_tool;

private slots:
	void slot_TOOL_NEW();
	void slot_TOOL_OPEN();
	void slot_TOOL_SAVE();
	void slot_TOOL_CLOSE();
	void slot_TOOL_COPY();
	void slot_TOOL_PASTE();
	void slot_TOOL_INSERT();
	void slot_TOOL_PLAY();
	void slot_TOOL_STOP();
	void slot_TOOL_INFO();
	void slot_TOOL_HAND();
	void slot_TOOL_SELECT_TIME();
	void slot_TOOL_SELECT_BOTH();
	void slot_TOOL_BANDPASS();
	void slot_TOOL_BANDPASS_ZERO();
	void slot_TOOL_BANDSUPPRESS();
	void slot_TOOL_PEN();
	void slot_TOOL_ERASER();
	void slot_TOOL_TIMESTEP(unsigned);
	void slot_TOOL_CURSOR_SYNC();

signals:
	void signalAction( ToolBar::ToolType _type, unsigned _value );

private:
	void connectQObject(QObject *_b, ToolType _t);
};

}

#endif

