#include "TemplateExportDialog.h"

#include <QPushButton>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QDir>
#include <QSplitter>
#include <QSyntaxHighlighter>

#include "../../SyntopiaCore/Logging/ListWidgetLogger.h"
#include "../../SyntopiaCore/Misc/Persistence.h"
#include "../Model/Rendering/TemplateRenderer.h"
#include "../../SyntopiaCore/Exceptions/Exception.h"

using namespace SyntopiaCore::Logging;
using namespace SyntopiaCore::Exceptions;
using namespace SyntopiaCore::Misc;

namespace StructureSynth {
	using namespace Model::Rendering;

	namespace GUI {


		namespace {

			class XmlHighlighter : public QSyntaxHighlighter {
			public:

				XmlHighlighter(QTextEdit* e) : QSyntaxHighlighter(e) {
					elementFormat.setForeground(Qt::blue);
					attributeFormat.setForeground(Qt::red);
					commentFormat.setForeground(Qt::darkGreen);
					cdataFormat.setForeground(Qt::darkBlue);
				};

				void highlightBlock(const QString &text)
				{

					
					if (previousBlockState() != 1 && currentBlockState() == 1) {
						// This line was previously a multi-line start 
						if (!text.contains("<!--")) setCurrentBlockState(0);
					}

					if (previousBlockState() == 1) {
						// Part of multi-line comment. Skip the rest...
						if (!text.contains("-->")) {
							setFormat(0, text.length(), commentFormat);
							setCurrentBlockState(1);
							return;
						}
					}

					if (previousBlockState() != 2 && currentBlockState() == 2) {
						if (!text.contains("<![CDATA[")) setCurrentBlockState(0);
					}

					if (previousBlockState() == 2) {
						// Part of multi-line comment. Skip the rest...
						if (!text.contains("]]>")) {
							setFormat(0, text.length(), cdataFormat);
							setCurrentBlockState(2);
							return;
						}
					}

					// Line parsing
					QString current;
					bool inElement = false;
					bool expectingAttribute = false;
					bool insideBracket = false;
					for (int i = 0; i < text.length(); i++) {
							
						if ((i >= 3) && (text.mid(i-3,4)=="<!--")) {
							// Multi-line comment begins
							setFormat(i-3, text.length()-(i-3), commentFormat);
							setCurrentBlockState(1);
							insideBracket = true;
						}

						if ((i >= 2) && (text.mid(i-2,3)=="-->")) {
							// Multi-line comment ends
							setFormat(0, i+1, commentFormat);
							if (currentBlockState() != 0) {
								setCurrentBlockState(0);
							}
							insideBracket = false;
						
							continue;
						}

						if ((i >= 8) && (text.mid(i-8,9)=="<![CDATA[")) {
							// Multi-line comment begins
							setFormat(i-8, text.length()-(i-8), cdataFormat);
							setCurrentBlockState(2);
							insideBracket = true;
						}

						if ((i >= 2) && (text.mid(i-2,3)=="]]>")) {
							// Multi-line comment ends
							setFormat(0, i+1, cdataFormat);
							if (currentBlockState() != 0) {
								setCurrentBlockState(0);
							}
							insideBracket = false;
						
							continue;
						}
						
						if (insideBracket) continue;
				
						current += text.at(i);
						if (text.at(i) == '<') {
							inElement = true;
							expectingAttribute = false; 
						}

						if (text.at(i) == ' ') expectingAttribute = true;
						if (text.at(i) == '=') expectingAttribute = false;
						
						
						if (inElement) {
							if (expectingAttribute) {
								setFormat(i, i+1, attributeFormat);
							} else {
								setFormat(i, i+1, elementFormat);
							}
						} else {
							setFormat(i, i+1, standardFormat);
						}

						if (text.at(i) == '>') inElement = false;

						
					}

				}; 
			private:
				QTextCharFormat elementFormat;
				QTextCharFormat cdataFormat;
				QTextCharFormat standardFormat;
				QTextCharFormat attributeFormat;
				QTextCharFormat commentFormat;

			};
		};




		TemplateExportDialog::TemplateExportDialog(QWidget* parent, QStringList primitives) : primitives(primitives), QDialog(parent) {
			// 'primitives' contain a list of used primitives.
			// We add 'begin' and 'end' since these are always used.
			this->primitives.append("begin");
			this->primitives.append("end");
			setupUi();
			retranslateUi();
		}

		TemplateExportDialog::~TemplateExportDialog() {
			// Persist (should only be done on OK?)
			Persistence::Store(fileNameLineEdit);
		}

		void TemplateExportDialog::setDefaultSize(int width, int height) {
			lockAspectRatioCheckBox->setChecked(false);
			heightSpinBox->setValue(height);
			widthSpinBox->setValue(width);
			aspectRatio = width/(double)height;
			lockAspectRatioCheckBox->setChecked(true);
			lockAspectRatioCheckBox->setText(QString("Lock aspect ratio (Current = %1)").arg(aspectRatio));

		}

		void TemplateExportDialog::lockAspectChanged() {
		}

		void TemplateExportDialog::heightChanged(int) {			
			if (lockAspectRatioCheckBox->isChecked()) {
				widthSpinBox->blockSignals(true);
				widthSpinBox->setValue((int)(aspectRatio*heightSpinBox->value()));
				INFO(QString("width set: %1").arg((int)(aspectRatio*heightSpinBox->value())));
				widthSpinBox->blockSignals(false);
			} else {
				aspectRatio = widthSpinBox->value() / (double)heightSpinBox->value();
				lockAspectRatioCheckBox->setText(QString("Lock aspect ratio (Current = %1)").arg(aspectRatio));
			}
		}

		void TemplateExportDialog::widthChanged(int) {
			if (lockAspectRatioCheckBox->isChecked()) {
				heightSpinBox->blockSignals(true);
				heightSpinBox->setValue((int)(widthSpinBox->value()/aspectRatio));
				INFO(QString("height set: %1").arg((int)(widthSpinBox->value()/aspectRatio)));
				heightSpinBox->blockSignals(false);			
			} else {
				aspectRatio = widthSpinBox->value() / (double)heightSpinBox->value();
				lockAspectRatioCheckBox->setText(QString("Lock aspect ratio (Current = %1)").arg(aspectRatio));	
			}
		}

		void TemplateExportDialog::setupUi()
		{
			if (objectName().isEmpty())
				setObjectName(QString::fromUtf8("Dialog"));
			resize(544, 600);
			setSizeGripEnabled(true);
			setModal(true);
			verticalLayout = new QVBoxLayout(this);
			verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
			horizontalLayout = new QHBoxLayout();
			horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
			label = new QLabel(this);
			label->setObjectName(QString::fromUtf8("label"));

			horizontalLayout->addWidget(label);

			templateComboBox = new QComboBox(this);
			templateComboBox->setObjectName(QString::fromUtf8("templateComboBox"));
			QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
			sizePolicy.setHorizontalStretch(0);
			sizePolicy.setVerticalStretch(0);
			sizePolicy.setHeightForWidth(templateComboBox->sizePolicy().hasHeightForWidth());
			templateComboBox->setSizePolicy(sizePolicy);
			connect(templateComboBox, SIGNAL(currentIndexChanged(const QString &)), 
				this, SLOT(templateChanged(const QString &)));


			horizontalLayout->addWidget(templateComboBox);

			templatePathButton = new QPushButton(this);
			templatePathButton->setObjectName(QString::fromUtf8("templatePathButton"));

			horizontalLayout->addWidget(templatePathButton);


			verticalLayout->addLayout(horizontalLayout);

			tabWidget = new QTabWidget(this);
			tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
			settingstab = new QWidget();
			settingstab->setObjectName(QString::fromUtf8("settingstab"));
			settingstab->setAutoFillBackground(false);

			verticalLayout_3 = new QVBoxLayout(settingstab);
			verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));

			QSplitter* splitter = new QSplitter(settingstab);
			verticalLayout_3->addWidget(splitter);


			QWidget* box1 = new QWidget(splitter);
			QVBoxLayout* box1Layout = new QVBoxLayout(box1);

			QWidget* box2 = new QWidget(splitter);
			QVBoxLayout* box2Layout = new QVBoxLayout(box2);

			splitter->addWidget(box1);
			splitter->addWidget(box2);
			QList<int> list;
			list.append(200);
			list.append(100);
			splitter->setSizes(list);



			descriptionLabel = new QLabel(settingstab);
			descriptionLabel->setObjectName(QString::fromUtf8("descriptionLabel"));
			box1Layout->addWidget(descriptionLabel);
			descriptionTextBrowser = new QTextBrowser(settingstab);
			descriptionTextBrowser->setObjectName(QString::fromUtf8("descriptionTextBrowser"));
			box1Layout->addWidget(descriptionTextBrowser);


			primitivesLabel = new QLabel(settingstab);
			primitivesLabel->setObjectName(QString::fromUtf8("primitivesLabel"));

			box2Layout->addWidget(primitivesLabel);

			primitivesTableWidget = new QTableWidget(settingstab);
			primitivesTableWidget->horizontalHeader()->setStretchLastSection(true);
			primitivesTableWidget->setObjectName(QString::fromUtf8("primitivesTableWidget"));

			box2Layout->addWidget(primitivesTableWidget);

			templateOutputGroupBox = new QGroupBox(settingstab);
			templateOutputGroupBox->setObjectName(QString::fromUtf8("templateOutputGroupBox"));
			templateOutputGroupBox->setFlat(true);
			verticalLayout_2 = new QVBoxLayout(templateOutputGroupBox);
			verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
			horizontalLayout_2 = new QHBoxLayout();
			horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
			fileRadioButton = new QRadioButton(templateOutputGroupBox);
			fileRadioButton->setObjectName(QString::fromUtf8("fileRadioButton"));
			fileRadioButton->setChecked(true);

			horizontalLayout_2->addWidget(fileRadioButton);

			fileNameLineEdit = new QLineEdit(templateOutputGroupBox);
			fileNameLineEdit->setObjectName(QString::fromUtf8("TemplateExportDialog.fileNameLineEdit"));
			fileNameLineEdit->setText(QApplication::translate("Dialog", "C:\\Output\\test.es", 0, QApplication::UnicodeUTF8));
			Persistence::Restore(fileNameLineEdit);


			horizontalLayout_2->addWidget(fileNameLineEdit);

			filePushButton = new QPushButton(templateOutputGroupBox);
			filePushButton->setObjectName(QString::fromUtf8("filePushButton"));
			QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
			sizePolicy1.setHorizontalStretch(0);
			sizePolicy1.setVerticalStretch(0);
			sizePolicy1.setHeightForWidth(filePushButton->sizePolicy().hasHeightForWidth());
			filePushButton->setSizePolicy(sizePolicy1);

			horizontalLayout_2->addWidget(filePushButton);


			verticalLayout_2->addLayout(horizontalLayout_2);

			horizontalLayout_3 = new QHBoxLayout();
			horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
			horizontalSpacer = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

			horizontalLayout_3->addItem(horizontalSpacer);

			uniqueCheckBox = new QCheckBox(templateOutputGroupBox);
			uniqueCheckBox->setObjectName(QString::fromUtf8("uniqueCheckBox"));

			horizontalLayout_3->addWidget(uniqueCheckBox);


			verticalLayout_2->addLayout(horizontalLayout_3);

			clipboardRadioButton = new QRadioButton(templateOutputGroupBox);
			clipboardRadioButton->setObjectName(QString::fromUtf8("clipboardRadioButton"));

			verticalLayout_2->addWidget(clipboardRadioButton);



			//--

			horizontalLayout_5 = new QHBoxLayout();
			horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
			label_4 = new QLabel(templateOutputGroupBox);
			label_4->setObjectName(QString::fromUtf8("label_4"));
			label_4->setMinimumSize(QSize(10, 10));

			horizontalLayout_5->addWidget(label_4);

			widthSpinBox = new QSpinBox(templateOutputGroupBox);
			widthSpinBox->setRange(0,20000);
			widthSpinBox->setObjectName(QString::fromUtf8("widthSpinBox"));

			horizontalLayout_5->addWidget(widthSpinBox);

			label_5 = new QLabel(templateOutputGroupBox);
			label_5->setObjectName(QString::fromUtf8("label_5"));

			horizontalLayout_5->addWidget(label_5);

			heightSpinBox = new QSpinBox(templateOutputGroupBox);
			heightSpinBox->setObjectName(QString::fromUtf8("heightSpinBox"));
			heightSpinBox->setRange(0,20000);
			connect(heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(heightChanged(int)));
			connect(widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(widthChanged(int)));

			horizontalLayout_5->addWidget(heightSpinBox);

			line = new QFrame(templateOutputGroupBox);
			line->setObjectName(QString::fromUtf8("line"));
			line->setMinimumSize(QSize(20, 2));
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);

			horizontalLayout_5->addWidget(line);

			lockAspectRatioCheckBox = new QCheckBox(templateOutputGroupBox);
			lockAspectRatioCheckBox->setObjectName(QString::fromUtf8("lockAspectRatioCheckBox"));
			//connect(lockAspectRatioCheckBox, SIGNAL(valueChanged()), this, SLOT(lockAspectChanged()));


			horizontalLayout_5->addWidget(lockAspectRatioCheckBox);

			horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

			horizontalLayout_5->addItem(horizontalSpacer_4);


			verticalLayout_2->addLayout(horizontalLayout_5);

			//--


			verticalLayout_3->addWidget(templateOutputGroupBox);

			postProcessingGroupBox = new QGroupBox(settingstab);
			postProcessingGroupBox->setObjectName(QString::fromUtf8("postProcessingGroupBox"));
			postProcessingGroupBox->setFlat(true);
			verticalLayout_4 = new QVBoxLayout(postProcessingGroupBox);
			verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
			runAfterCheckBox = new QCheckBox(postProcessingGroupBox);
			runAfterCheckBox->setObjectName(QString::fromUtf8("runAfterCheckBox"));

			verticalLayout_4->addWidget(runAfterCheckBox);

			horizontalLayout_4 = new QHBoxLayout();
			horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
			horizontalSpacer_2 = new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

			horizontalLayout_4->addItem(horizontalSpacer_2);

			afterCommandLineEdit = new QLineEdit(postProcessingGroupBox);
			afterCommandLineEdit->setObjectName(QString::fromUtf8("afterCommandLineEdit"));

			horizontalLayout_4->addWidget(afterCommandLineEdit);


			verticalLayout_4->addLayout(horizontalLayout_4);

			runAfterCheckBox->raise();
			afterCommandLineEdit->raise();
			runAfterCheckBox->raise();
			runAfterCheckBox->raise();

			verticalLayout_3->addWidget(postProcessingGroupBox);

			tabWidget->addTab(settingstab, QString());
			advancedTab = new QWidget();
			advancedTab->setObjectName(QString::fromUtf8("advancedTab"));
			verticalLayout_5 = new QVBoxLayout(advancedTab);
			verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));

			modifyTemplateCheckBox = new QCheckBox(advancedTab);
			modifyTemplateCheckBox->setObjectName(QString::fromUtf8("modifyTemplateCheckBox"));

			verticalLayout_5->addWidget(modifyTemplateCheckBox);

			templateTextEdit = new QTextEdit(advancedTab);
			templateTextEdit->setTabStopWidth(30);
			templateTextEdit->setObjectName(QString::fromUtf8("templateTextEdit"));
			/*XmlHighlighter *highlighter =*/ new XmlHighlighter(templateTextEdit);

			verticalLayout_5->addWidget(templateTextEdit);

			horizontalLayout_6 = new QHBoxLayout();
			horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
			horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

			horizontalLayout_6->addItem(horizontalSpacer_3);

			pushButton_3 = new QPushButton(advancedTab);
			pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

			horizontalLayout_6->addWidget(pushButton_3);

			pushButton_4 = new QPushButton(advancedTab);
			pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));

			horizontalLayout_6->addWidget(pushButton_4);


			verticalLayout_5->addLayout(horizontalLayout_6);

			modifyOutputCheckBox = new QCheckBox(advancedTab);
			modifyOutputCheckBox->setObjectName(QString::fromUtf8("modifyOutputCheckBox"));

			verticalLayout_5->addWidget(modifyOutputCheckBox);

			tabWidget->addTab(advancedTab, QString());

			verticalLayout->addWidget(tabWidget);

			buttonBox = new QDialogButtonBox(this);
			buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
			buttonBox->setOrientation(Qt::Horizontal);
			buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

			verticalLayout->addWidget(buttonBox);


			retranslateUi();
			QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
			QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

			tabWidget->setCurrentIndex(0);


			//QMetaObject::connectSlotsByName(this);
		} // setupUi

		void TemplateExportDialog::setTemplatePath(QString templatePath) {

			// Scan render templates...
			QDir dir(templatePath);
			QStringList filters;
			filters << "*.rendertemplate";
			dir.setNameFilters(filters);
			if (!dir.exists()) {
				WARNING("Unable to locate: "+dir.absolutePath());
			} else {
				QStringList sl = dir.entryList();
				templateComboBox->clear();
				for (int i = 0; i < sl.size(); i++) {
					templateComboBox->insertItem(10000, sl[i].remove(".rendertemplate", Qt::CaseInsensitive), 
						dir.absoluteFilePath(sl[i]));
				}
			}


		}

		void TemplateExportDialog::templateChanged(const QString &) {
			int id = templateComboBox->currentIndex();
			if (id<0) return;
			QVariant q = templateComboBox->itemData(id);

			QFile file(q.toString());
			
			primitivesTableWidget->setColumnCount(1);
			primitivesTableWidget->horizontalHeader()->hide();
				
			try {
				Template t(file);

				templateTextEdit->setText(t.getFullText());
				descriptionTextBrowser->setText(t.getDescription());

				primitivesTableWidget->setRowCount(t.getPrimitives().count());
				QMapIterator<QString, TemplatePrimitive> i(t.getPrimitives());
				int count = 0;
				while (i.hasNext()) {
					i.next();
					QTableWidgetItem* item = new QTableWidgetItem( i.key());
					if (primitives.contains(i.key())) {
						item->setBackground(QBrush(Qt::green));
					}
					primitivesTableWidget->setItem(count, 0, item);

					count++;
				}
			} catch (Exception& e) {
				primitivesTableWidget->setRowCount(0);
				
				WARNING(e.getMessage());
				//templateTextEdit->setText(e.getMessage());	
				descriptionTextBrowser->setText(e.getMessage());
			}

			
		}





		void TemplateExportDialog::retranslateUi()
		{
			setWindowTitle(QApplication::translate("Dialog", "Template Export", 0, QApplication::UnicodeUTF8));
			label->setText(QApplication::translate("Dialog", "Template:", 0, QApplication::UnicodeUTF8));
			templatePathButton->setText(QApplication::translate("Dialog", "Template Path...", 0, QApplication::UnicodeUTF8));
			descriptionLabel->setText(QApplication::translate("Dialog", "Description", 0, QApplication::UnicodeUTF8));
			descriptionTextBrowser->setHtml(QApplication::translate("Dialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
				"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
				"p, li { white-space: pre-wrap; }\n"
				"</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
				"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">This template was created by ....</span></p></body></html>", 0, QApplication::UnicodeUTF8));
			primitivesLabel->setText(QApplication::translate("Dialog", "Primitives in Template", 0, QApplication::UnicodeUTF8));
			templateOutputGroupBox->setTitle(QApplication::translate("Dialog", "Template Output", 0, QApplication::UnicodeUTF8));
			fileRadioButton->setText(QApplication::translate("Dialog", "File:", 0, QApplication::UnicodeUTF8));
			filePushButton->setText(QApplication::translate("Dialog", "File...", 0, QApplication::UnicodeUTF8));
			uniqueCheckBox->setText(QApplication::translate("Dialog", "Add unique ID to filename (testmig-1.es)", 0, QApplication::UnicodeUTF8));
			clipboardRadioButton->setText(QApplication::translate("Dialog", "Clipboard", 0, QApplication::UnicodeUTF8));
			postProcessingGroupBox->setTitle(QApplication::translate("Dialog", "Post Processing", 0, QApplication::UnicodeUTF8));
			runAfterCheckBox->setText(QApplication::translate("Dialog", "Run the following command after export:", 0, QApplication::UnicodeUTF8));
			afterCommandLineEdit->setText(QApplication::translate("Dialog", "C:\\sunflow\\sunflow.bat $FILE", 0, QApplication::UnicodeUTF8));
			tabWidget->setTabText(tabWidget->indexOf(settingstab), QApplication::translate("Dialog", "Settings", 0, QApplication::UnicodeUTF8));
			label_4->setText(QApplication::translate("Dialog", "Width:", 0, QApplication::UnicodeUTF8));
			label_5->setText(QApplication::translate("Dialog", "Height:", 0, QApplication::UnicodeUTF8));
			lockAspectRatioCheckBox->setText(QApplication::translate("Dialog", "Lock aspect ratio (Current = 1.23)", 0, QApplication::UnicodeUTF8));
			modifyTemplateCheckBox->setText(QApplication::translate("Dialog", "Modify template before applying", 0, QApplication::UnicodeUTF8));
			pushButton_3->setText(QApplication::translate("Dialog", "Save Modifications", 0, QApplication::UnicodeUTF8));
			pushButton_4->setText(QApplication::translate("Dialog", "Revert (Undo Changed)", 0, QApplication::UnicodeUTF8));
			modifyOutputCheckBox->setText(QApplication::translate("Dialog", "Modify output before saving (spawns edit window when pressing OK)", 0, QApplication::UnicodeUTF8));
			tabWidget->setTabText(tabWidget->indexOf(advancedTab), QApplication::translate("Dialog", "Modify", 0, QApplication::UnicodeUTF8));

		} // retranslateUi

	};


}
