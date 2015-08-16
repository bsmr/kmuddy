/********************************************f*******************************
                             cmapzonemanager.cpp
                             -------------------
    begin                : Sat Aug 15, 2015
    copyright            : (C) 2015 Tomas Mecir
    email                : mecirt@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cmapzonemanager.h"

#include <QStandardItemModel>
#include <QDir>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "cmapmanager.h"
#include "cprofilemanager.h"

struct CMapZoneManager::Private {
  QStandardItemModel zones;
  int sessId;
  CMapManager *manager;
  int currentZone;
};


CMapZoneManager::CMapZoneManager(int sessId, CMapManager *manager)
{
  d = new Private;
  d->sessId = sessId;
  d->manager = manager;
  d->currentZone = -1;
  loadMapList();
}

CMapZoneManager::~CMapZoneManager()
{
  for (int i = 0; i < d->zones.rowCount(); ++i)
    delete getZoneEntry(i);

  delete d;
}


QStandardItemModel *CMapZoneManager::zonesModel()
{
  return &d->zones;
}

QString CMapZoneManager::getPath(int idx)
{
  cZoneInformation *i = getZoneEntry(idx);
  if (!i) return QString();

  cProfileManager *pm = cProfileManager::self();
  if (!pm->settings(d->sessId)) return QString();
  
  QString path = pm->profilePath (d->sessId) + "/maps/";
  QDir dir = QDir (path);
  if (!dir.exists()) QDir::root().mkpath (dir.absolutePath());
  return path + "/" + i->file;
}

void CMapZoneManager::loadZone(int idx)
{
  if (d->currentZone == idx) return;

  // first save the current zone
  QString savePath = getPath (d->currentZone);
  if (savePath.length()) {
    CMapFileFilterBase *filter = d->manager->nativeFilter(false);
    if (filter && (!d->manager->isClean())) {
      // TODO: hmm, maybe simply always save by default?
      if (KMessageBox::warningYesNo (NULL, i18n("Do you want to save the changes to the current map?"), i18n("KMuddy Mapper")) == KMessageBox::Yes)
        d->manager->exportMap (savePath, filter);
    }
  }
  
  // and load, replacing the existing data
  QString loadPath = getPath(idx);
  CMapFileFilterBase *filter = d->manager->nativeFilter(true);
  if (filter) {
    d->currentZone = idx;
    d->manager->importMap (loadPath, filter);
  }
}

void CMapZoneManager::deleteZone(int idx)
{
  cZoneInformation *i = getZoneEntry(idx);
  if (!i) return;

  // wipe the data, if loaded
  if (idx == d->currentZone)
    d->manager->eraseMap();

  // wipe the file
  QString fName = getPath(idx);
  QFile f(fName);
  f.remove();

  // wipe the record and the model entry
  d->zones.removeRows(idx, 1);
  delete i;

  // and load whichever zone is next, if this one was the current one
  // TODO: won't this happen automatically?

  if (!d->zones.rowCount())
    createZone (i18n ("Map #1"));

  saveMapList();
}

void CMapZoneManager::createZone(const QString &name, const QString &file)
{
  // generate file name if needed
  QString fName = file;
  if (!fName.length()) {
    cProfileManager *pm = cProfileManager::self();
    if (!pm->settings(d->sessId)) fName = "map.xml";
    else
    {
      int id = 1;
      QString path = pm->profilePath (d->sessId) + "/maps/";
      do {
        QFile f(path + QString::number(id) + ".xml");
        if (!f.exists()) break;
        id++;
      } while (1);
      fName = QString::number(id) + ".xml";
    }
  }

  int id = createZoneEntry(name, fName);
  loadZone(id);

  saveMapList();
}

void CMapZoneManager::renameZone(int idx, const QString &name)
{
  cZoneInformation *i = getZoneEntry(idx);
  if (i) i->name = name;
}

int CMapZoneManager::createZoneEntry(const QString &name, const QString &file)
{
  cZoneInformation *i = new cZoneInformation;
  i->name = name;
  i->file = file;

  QStandardItem *item = new QStandardItem(name);
  item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  item->setData(QVariant(qVariantFromValue((void *) i)), Qt::UserRole + 1);

  d->zones.appendRow(item);
  return d->zones.rowCount() - 1;
}

cZoneInformation *CMapZoneManager::getZoneEntry(int idx)
{
  QModelIndex index = d->zones.index(idx, 0);
  if (!index.isValid()) return 0;
  cZoneInformation *i = static_cast<cZoneInformation*>(index.data(Qt::UserRole + 1).value<void *>());
  return i;
}

int CMapZoneManager::activeZone() {
  return d->currentZone;
}

void CMapZoneManager::loadMapList()
{
  cProfileManager *pm = cProfileManager::self();
  if (!pm->settings(d->sessId)) return;

  d->zones.clear();
  QString path = pm->profilePath (d->sessId);
  QDir dir = QDir (path);
  if (!dir.exists()) QDir::root().mkpath (dir.absolutePath());
  QString fName = "maps.xml";

  QFile f (path + "/" + fName);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    kDebug() << "No " << fName << " file - nothing to do." << endl;
    return;
  }

  QXmlStreamReader *reader = new QXmlStreamReader (&f);

  reader->readNext ();  // read the document start
  reader->readNext ();
  if (reader->isStartElement ())
    if (reader->name() == "maps")
      if (reader->attributes().value ("version") == "1.0") {
        // all is well, we can start loading the list
        while (!reader->atEnd()) {
          reader->readNext ();
    
          if (reader->isEndElement() && (reader->name() == "maps")) break;

          if (reader->isStartElement () && (reader->name() == "map")) {
            QString name = reader->attributes().value ("name").toString();
            QString file = reader->attributes().value ("file").toString();
            createZoneEntry(name, file);
          }
        }
      }
      else
        reader->raiseError (i18n ("This file was created by a newer version of KMuddy, and this version is unable to open it."));
    else
      reader->raiseError (i18n ("This is not a KMuddy map list."));
  else if (!reader->hasError())
    reader->raiseError (i18n ("This file is corrupted."));

  if (reader->hasError()) {
    kDebug() << i18n ("Error at line %1, column %2: %3",
                            QString::number (reader->lineNumber()),
                            QString::number (reader->columnNumber()),
                            reader->errorString());
  }

  f.close ();
  delete reader;
}


void CMapZoneManager::saveMapList()
{
  // save the list contents
  cProfileManager *pm = cProfileManager::self();
  if (!pm->settings(d->sessId)) return;

  QString path = pm->profilePath (d->sessId);
  QDir dir = QDir (path);
  if (!dir.exists()) QDir::root().mkpath (dir.absolutePath());
  QString fName = "maps.xml";

  dir.remove ("maps.backup");
  if (!QFile(path + "/" + fName).copy (path + "/" + "maps.backup")) {
    kDebug() << "Unable to backup " << fName << endl;  // not fatal, may simply not exist
  }

  QFile f (path + "/" + fName);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
    kDebug() << "Unable to open " << (path + "/" + fName) << " for writing." << endl;
    return;  // problem
  }

  QXmlStreamWriter *writer = new QXmlStreamWriter (&f);
  writer->setAutoFormatting (true);  // make the generated XML more readable

  writer->writeStartDocument ();
  writer->writeStartElement ("maps");
  writer->writeAttribute ("version", "1.0");

  for (int i = 0; i < d->zones.rowCount(); ++i) {
    cZoneInformation *zi = getZoneEntry(i);
    if (!zi) continue;

    writer->writeStartElement ("map");
    writer->writeAttribute ("name", zi->name);
    writer->writeAttribute ("file", zi->file);
    writer->writeEndElement ();  // end the map element
  }

  writer->writeEndElement ();  // end the maps element
  writer->writeEndDocument ();

  f.close ();
  delete writer;
}
