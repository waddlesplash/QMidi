/*
 * Copyright 2019 Georg Gadinger <nilsding@nilsding.org>
 * Distributed under the terms of the MIT license.
 */
#include "QMidiIn.h"

QMidiIn::QMidiIn(QObject *parent)
	: QObject(parent),
	fMidiPtrs(nullptr),
	fConnected(false)
{
}

QMidiIn::~QMidiIn()
{
	if (fConnected)
		disconnect();
}
