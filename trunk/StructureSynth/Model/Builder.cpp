#include "Builder.h"
#include "../../SyntopiaCore/Logging/Logging.h"
#include "../../SyntopiaCore/Exceptions/Exception.h"
#include "../../SyntopiaCore/Misc/ColorUtils.h"
#include "../../SyntopiaCore/Math/Vector3.h"

#include <QProgressDialog>
#include <QApplication>

using namespace SyntopiaCore::Logging;
using namespace SyntopiaCore::Math;
using namespace SyntopiaCore::Exceptions;

namespace StructureSynth {
	namespace Model {

		Builder::Builder(Rendering::Renderer* renderTarget, RuleSet* ruleSet) : renderTarget(renderTarget), ruleSet(ruleSet) {
			maxGenerations = 1000;
			maxObjects = 100000;
			objects = 0;


	
		};
			

		void Builder::build() {
			objects = 0;
			INFO("Starting builder...");
			
			/// Push first generation state
			stack.currentStack.append(RuleState(ruleSet->getStartRule(), State()));
			int generationCounter = 0;
			
			QProgressDialog progressDialog("Building objects...", "Cancel", 0, 100, 0);
			progressDialog.setWindowModality(Qt::WindowModal);
			progressDialog.show();

			int lastValue = 0;

			while (generationCounter < maxGenerations && objects < maxObjects) {


				double p = 0;
				if (maxObjects>0) {
					p = objects/(double)maxObjects;
				}

				double p2 = 0;
				if (maxGenerations>0) {
					p2 = generationCounter/(double)maxGenerations;
				}

				double progress = p;
				if (p2 > p) progress = p2;

				if (maxObjects<=0 && maxGenerations<=0) {
					progress = (generationCounter%9)/9.0;
				}

				if (lastValue != (int)(progress*100.0)) {
					progressDialog.setValue((int)(progress*100.0));
					progressDialog.setLabelText(
						QString("Building objects...\r\n\r\nGeneration: %1\r\nObjects: %2\r\nPending rules: %3")
						.arg(generationCounter).arg(objects).arg(stack.currentStack.size()));
					qApp->processEvents();
				}

				lastValue = (int)(progress*100.0);
				
				if (progressDialog.wasCanceled()) {
					break;
				}

				generationCounter++;

				// Now iterate though all RuleState's on stack and create next generation.
				//INFO(QString("Executing generation %1 with %2 individuals").arg(generationCounter).arg(stack.currentStack.size()));
				nextStack.currentStack.clear();
				for (int i = 0; i < stack.currentStack.size(); i++) {
					//	INFO("Executing: " + stack.currentStack[i].rule->getName());
					state = stack.currentStack[i].state;
					stack.currentStack[i].rule->apply(this);
				}
				stack = nextStack;

				if (stack.currentStack.size() == 0) break; // no need to continue...
			}

			progressDialog.setValue(100); 
			progressDialog.hide();

			if (progressDialog.wasCanceled()) {
				INFO("User terminated.");
			}

			if (objects == maxObjects) {
				INFO(QString("Terminated because maximum number of objects reached (%1).").arg(maxObjects));
				INFO(QString("Use 'Set MaxObjects' command to increase this number."));
			}
			
			if (generationCounter == maxGenerations) {
				INFO(QString("Terminated because maximum number of generations reached (%1).").arg(maxGenerations));
				INFO(QString("Use 'Set Maxdepth' command to increase this number."));
			}
			INFO("Done building...");
		}

		void Builder::setCommand(QString command, QString param) {
			if (command == "maxdepth") {
				//INFO(QString("Setting 'maxgenerations' to %1").arg(param));
				bool succes;
				int i = param.toInt(&succes);
				if (!succes) throw Exception(QString("Command 'maxdepth' expected integer parameter. Found: %1").arg(param));
				maxGenerations = i;
			} else if (command == "maxobjects") {
				//INFO(QString("Setting 'maxgenerations' to %1").arg(param));
				bool succes;
				int i = param.toInt(&succes);
				if (!succes) throw Exception(QString("Command 'maxobjects' expected integer parameter. Found: %1").arg(param));
				maxObjects = i;
			} else if (command == "seed") {
				bool succes;
				int i = param.toInt(&succes);
				if (!succes) throw Exception(QString("Command 'seed' expected integer parameter. Found: %1").arg(param));
				srand(i);
			} else if (command == "background") {
				QColor c(param);
				if (!c.isValid()) throw Exception(QString("Command 'background' expected a valid color identifier: Found: %1").arg(param));
				renderTarget->setBackgroundColor(Vector3f(c.red() / 255.0, c.green() / 255.0, c.blue() / 255.0) );
			} else {
				throw Exception(QString("Unknown command: %1").arg(command));
			}
		}
		
		ExecutionStack& Builder::getExecutionStack() {
			return nextStack;
		}

	}
}

