//
// C++ Interface: cgaugebaritem
//
// Description: 
//
/*
Copyright 2004-2011 Tomas Mecir <kmuddy@kmuddy.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CGAUGEBARITEM_H
#define CGAUGEBARITEM_H

#include <qwidget.h>

//the "real" gauge displayer
class cGaugeBarItemPrivate;
class QLabel;

/**
One item in the gauge bar. Contains a label and the gauge.

@author Tomas Mecir
*/
class cGaugeBarItem : public QWidget
{
Q_OBJECT
public:
  /** constructor */
  cGaugeBarItem (QWidget *parent = 0);
  /** destructor */
  ~cGaugeBarItem ();
  void setText (const QString &caption);
  QString text () const;
  void setValue (int value);
  int value () const;
  void setColor (const QColor &color);
  QColor color () const;
protected:
  QLabel *label;
  cGaugeBarItemPrivate *gaugepriv;
};

class cGaugeBarItemPrivate: public QWidget
{
Q_OBJECT
public:
  /** constructor */
  cGaugeBarItemPrivate (QWidget *parent = 0);
  /** destructor */
  ~cGaugeBarItemPrivate ();
  void setValue (int value);
  int value () const;
  void setColor (const QColor &color);
  QColor color () const;

  virtual QSize sizeHint () const;
protected:
  void paintEvent (QPaintEvent *);

  int val;
  QColor col;
};

#endif
