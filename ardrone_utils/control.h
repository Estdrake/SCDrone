#ifndef CONTROL_H
#define CONTROL_H

#include "at_client.h"
#include "at_cmd.h"

#include <iostream>

#include <QtGamepad/QGamepad>
#include <QLoggingCategory>
#include <QCoreApplication>

#include <thread>
#include <chrono>
class GamepadMonitor : public QObject
{
private:
	QGamepad* gamepad;
	ATQueue* queue;

public:
	GamepadMonitor(ATQueue* queue,QCoreApplication* a,ATClient* client,QObject* parent = 0) : QObject(a), gamepad(0)
	{
		this->queue = queue;

		QLoggingCategory::setFilterRules(QStringLiteral("qt.gamepad.debug=true"));
		QGamepadManager* manager = QGamepadManager::instance();
		QList<int> gamepads;
		int i = 0;
		while(i < 100) {
			a->processEvents();
			gamepads = manager->connectedGamepads();
			if(!gamepads.isEmpty())
			{
				qInfo() << "gamepad found";
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			i++;
		}
		if(gamepads.isEmpty())
		{
			qDebug() << "No gamepad found";
			return;
		}

		gamepad = new QGamepad(*gamepads.begin(), this);
		connect(gamepad, &QGamepad::axisLeftXChanged, this, [&](double value) {
			qDebug() << "Left X" << value;
			queue->push(at_format_pcmd(PROGRESSIVE, value, 0, 0, 0));
		});
		connect(gamepad, &QGamepad::axisLeftYChanged, this, [&](double value) {
			qDebug() << "Left Y" << value;
			queue->push(at_format_pcmd(PROGRESSIVE, 0, value, 0, 0));
		});
		connect(gamepad, &QGamepad::axisRightXChanged, this, [&](double value) {
			qDebug() << "Right X" << value;
			queue->push(at_format_pcmd(PROGRESSIVE, 0, 0, 0, value));
		});
		connect(gamepad, &QGamepad::axisRightYChanged, this, [&](double value) {
			qDebug() << "Right Y" << value;
			queue->push(at_format_pcmd(PROGRESSIVE, 0, 0, value, 0));
		});
		connect(gamepad, &QGamepad::buttonAChanged, this, [&](bool pressed) {
			qDebug() << "Button A" << pressed;
		});
		connect(gamepad, &QGamepad::buttonBChanged, this, [&](bool pressed) {
			qDebug() << "Button B" << pressed;
		});
		connect(gamepad, &QGamepad::buttonXChanged, this, [&](bool pressed) {
			qDebug() << "Button X" << pressed;
		});

		connect(gamepad, &QGamepad::buttonYChanged, this, [&](bool pressed) {
			qDebug() << "Button Y" << pressed;
		});
		connect(gamepad, &QGamepad::buttonL1Changed, this, [&](bool pressed) {
			qDebug() << "Button L1" << pressed;
		});
		connect(gamepad, &QGamepad::buttonR1Changed, this, [&](bool pressed) {
			qDebug() << "Button R1" << pressed;
		});
		connect(gamepad, &QGamepad::buttonL2Changed, this, [&](double value) {
			qDebug() << "Button L2: " << value;
		});
		connect(gamepad, &QGamepad::buttonR2Changed, this, [&](double value) {
			qDebug() << "Button R2: " << value;
		});
		connect(gamepad, &QGamepad::buttonSelectChanged, this, [&](bool pressed) {
			qDebug() << "Button Select" << pressed;
			if(pressed == true)
			{
				client->set_ref(LAND_FLAG);
			}
		});
		connect(gamepad, &QGamepad::buttonStartChanged, this, [&](bool pressed) {
			qDebug() << "Button Start" << pressed;
			if(pressed == true)
			{
				client->set_ref(TAKEOFF_FLAG);
			}
		});
		connect(gamepad, &QGamepad::buttonGuideChanged, this, [](bool pressed) {
			qDebug() << "Button Guide" << pressed;
		});
	}
};



#endif