#ifndef QTDASHERIMPL_H
#define QTDASHERIMPL_H

#include <qpe/inputmethodinterface.h>
#include "QtDasherPlugin.h"

class QtDasher;
class QPixmap;

class QtDasherImpl : public InputMethodInterface
{
public:
    QtDasherImpl();
    virtual ~QtDasherImpl();

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif

    virtual QWidget *inputMethod( QWidget *parent, Qt::WFlags f );
    virtual void resetState();
    virtual QPixmap *icon();
    virtual QString name();
    virtual void onKeyPress( QObject *receiver, const char *slot );

private:
    CDasherInterface *qtdasherinterface;
    QtDasherPlugin *qtdasherwidget;
    QPixmap *icn;
    ulong ref;
};

#endif




