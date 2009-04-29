#pragma once

#include <QMainWindow>
#include <QTabBar>
#include <QStackedWidget>


#include "../../SyntopiaCore/GLEngine/EngineWidget.h"
#include "../../SyntopiaCore/Misc/Version.h"
#include "VariableEditor.h"



class QAction;
class QMenu;
class QTextEdit;

namespace StructureSynth {
	namespace GUI {
 
		struct TabInfo {
			TabInfo() {}; 
			TabInfo(QString filename, QTextEdit* textEdit) : filename(filename), unsaved(false), textEdit(textEdit), hasBeenSavedOnce(false) {};
			TabInfo(QString filename, QTextEdit* textEdit, bool unsaved, bool hasBeenSavedOnce=false) : filename(filename), unsaved(unsaved), textEdit(textEdit), hasBeenSavedOnce(hasBeenSavedOnce) {};
			QString filename;
			bool unsaved;
			QTextEdit* textEdit;
			bool hasBeenSavedOnce;			
        };


		/// The main window of the application.
		/// As of now a SDI interface, but at some point tabs will be added.
		class MainWindow : public QMainWindow
		{
			Q_OBJECT

		public:
			MainWindow();
			MainWindow(const QString &fileName);
			void setSeed(int randomSeed);
			int getSeed();

			static QString getExamplesDir();
			static QString getMiscDir();
			static QString getTemplateDir();
			
		protected:
			void dragEnterEvent(QDragEnterEvent *ev);
			void dropEvent(QDropEvent *ev);
			void closeEvent(QCloseEvent* ev);
			void keyReleaseEvent(QKeyEvent* ev);

		private slots:
			void closeTab(int id);
			void cut();
			void copy();
			void paste();
			void insertCameraSettings();
			void cursorPositionChanged();
			void seedChanged();
			void tabChanged(int index);
			void makeScreenshot();
			void closeTab();
			void launchSfHome();
			void fastRotateChanged();
			void launchGallery();
			void launchReferenceHome();
			void templateRender();
			void templateRenderToFile();
			void templateRender(const QString& fileName);
			
			void openFile();
			void newFile();
			void open();
			bool save();
			bool saveAs();
			void about();
			void documentWasModified();
			void render();
			void resetView();
			void toggleFullScreen();
			

		private:
			void parseJavaScript(QString scripture);
			void insertTabPage(QString filename);
			QTextEdit* getTextEdit();
			void init();
			void createActions();
			void createMenus();
			void createToolBars();
			void createStatusBar();
			void readSettings();
			void writeSettings();
			void updateRandom();
			void loadFile(const QString &fileName);
			bool saveFile(const QString &fileName);
			QString strippedName(const QString &fullFileName);
			void createOpenGLContextMenu();

			QSpinBox* seedSpinBox;
			QCheckBox* autoIncrementCheckbox;

			QDockWidget* dockLog;
			QAction* fullScreenAction;
			QAction* insertCameraSettingsAction;
			QAction* screenshotAction;
			QAction* sfHomeAction;
			QAction* referenceAction;
			QAction* galleryAction;
			QMenu *fileMenu;
			QMenu *editMenu;
			QMenu *renderMenu;
			QMenu *helpMenu;
			QToolBar *fileToolBar;
			QToolBar *renderToolBar;
			QToolBar *editToolBar;
			QToolBar *randomToolBar;
			QAction *newAction;
			QAction *openAction;
			QAction *saveAction;
			QAction *saveAsAction;
			QAction *closeAction;
			QAction *exitAction;
			QAction *cutAction;
			QAction *copyAction;
			QAction *pasteAction;
			QAction *aboutAction;

			QAction *renderAction;
			QAction *panicAction;
			SyntopiaCore::GLEngine::EngineWidget* engine;
			QTabBar* tabBar;

			SyntopiaCore::Misc::Version version;

			QMenu* openGLContextMenu;
			bool fullScreenEnabled;
			QStackedWidget *stackedTextEdits;

			QVector<TabInfo> tabInfo;

			int oldDirtyPosition;

			QVBoxLayout* frameMainWindow;
			QCheckBox* fastRotateCheckbox;
			VariableEditor* variableEditor;
			QDockWidget* editorDockWidget;

		};

	}
}
