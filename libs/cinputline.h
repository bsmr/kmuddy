/***************************************************************************
                          cinputline.h  -  input line widgets
    This file is a part of KMuddy distribution.
                             -------------------
    begin                : So Jun 29 2002
    copyright            : (C) 2002 by Tomas Mecir
    email                : kmuddy@kmuddy.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CINPUTLINE_H
#define CINPUTLINE_H

#include <klineedit.h>

#include <cactionbase.h>
#include <kmuddy_export.h>

#define CMDHISTORYSIZE 100

/**Input line - that's where you type your commands...
  *@author Tomas Mecir
  */

class KMUDDY_EXPORT cInputLine : public KLineEdit, public cActionBase  {
   Q_OBJECT
public: 
  cInputLine (int sess, QString objName="inputline", QWidget *parent=0);
  ~cInputLine();
  /** initialize the input line - must be separated
  from the constructor, because it uses cSession, which is not
  available in constructor */
  void initialize ();
  
  void keepText (bool value);
  void selectKeptText (bool value);
  void setArrowsHistory (bool value);
  void setAC (bool useac);
/** set auto-completion type. Types are defined in KGlobalSettings:
  CompletionNone=1      //No completion is used.
  CompletionAuto=2      //Text is automatically filled in whenever possible.
  CompletionMan=3       //Same as automatic except shortest match is used for completion
  CompletionShell=4     //Complete text much in the same way as a typical *nix shell would.
  CompletionPopup=5     //Lists all possible matches in a popup list-box to choose from.
  CompletionPopupAuto=6 //Lists all possible matches in a popup list-box to choose from, and automatically * fill the result whenever possible.
    Types 4 and 6 don't work. I have no idea why...
*/
  void setACType (int typeofac);
  void setTelnetPaste (bool tnp);
public slots:
  /** reimplemented handler for pasting */
  virtual void paste ();
protected slots:
  void handleEnter (const QString &text);
protected:
  virtual void eventNothingHandler (QString event, int session);
  virtual QString actionStringHandler (QString action, int, QString &par1,
      const QString &);

 // virtual void paste ();
  virtual void paste (const QString &t);
  void handleTabExpansion ();
  /** event filter - handles TAB expansion */
  virtual bool event (QEvent *e);
  /** keypress event - handles history Up/Down browsing*/
  void keyPressEvent (QKeyEvent *e);
  /** mouse-release event - handles middle-click paste */
  void mouseReleaseEvent (QMouseEvent *e);
  /** we got focus - restore selection (needed because selection is lost
    when switching between connections) */
  void focusInEvent (QFocusEvent *e);
  /** we lost focus - store selection (needed because selection is lost
    when switching between connections)*/
  void focusOutEvent (QFocusEvent *e);
  QString getHistory (bool next);
  QString menuitem[CMDHISTORYSIZE];
  void addHistory (const QString &text);
  int menuitems;
  int lastid;
  bool keeptext;
  bool selectkepttext;
  bool arrowshistory;
  bool useac;
  int curactype;
  int historypos;
  /** telnet-style paste */
  bool tnpaste;

  /** TAB-expansion */
  QStringList tabWords;
  bool tabExpanding;
  int expandPos, tabListPos;

  /** selection start/length, stored/restored in focusIN/focusOut events */
  int ss, sl;
};

#endif
