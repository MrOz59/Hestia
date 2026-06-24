/****************************************************************************
** Meta object code from reading C++ file 'session.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../streaming/session.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'session.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Session_t {
    QByteArrayData data[21];
    char stringdata0[254];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Session_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Session_t qt_meta_stringdata_Session = {
    {
QT_MOC_LITERAL(0, 0, 7), // "Session"
QT_MOC_LITERAL(1, 8, 13), // "stageStarting"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 5), // "stage"
QT_MOC_LITERAL(4, 29, 11), // "stageFailed"
QT_MOC_LITERAL(5, 41, 9), // "errorCode"
QT_MOC_LITERAL(6, 51, 12), // "failingPorts"
QT_MOC_LITERAL(7, 64, 17), // "connectionStarted"
QT_MOC_LITERAL(8, 82, 18), // "displayLaunchError"
QT_MOC_LITERAL(9, 101, 4), // "text"
QT_MOC_LITERAL(10, 106, 12), // "quitStarting"
QT_MOC_LITERAL(11, 119, 15), // "sessionFinished"
QT_MOC_LITERAL(12, 135, 14), // "portTestResult"
QT_MOC_LITERAL(13, 150, 16), // "readyForDeletion"
QT_MOC_LITERAL(14, 167, 21), // "launchWarningsChanged"
QT_MOC_LITERAL(15, 189, 10), // "initialize"
QT_MOC_LITERAL(16, 200, 13), // "QQuickWindow*"
QT_MOC_LITERAL(17, 214, 8), // "qtWindow"
QT_MOC_LITERAL(18, 223, 5), // "start"
QT_MOC_LITERAL(19, 229, 9), // "interrupt"
QT_MOC_LITERAL(20, 239, 14) // "launchWarnings"

    },
    "Session\0stageStarting\0\0stage\0stageFailed\0"
    "errorCode\0failingPorts\0connectionStarted\0"
    "displayLaunchError\0text\0quitStarting\0"
    "sessionFinished\0portTestResult\0"
    "readyForDeletion\0launchWarningsChanged\0"
    "initialize\0QQuickWindow*\0qtWindow\0"
    "start\0interrupt\0launchWarnings"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Session[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       1,   94, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       4,    3,   72,    2, 0x06 /* Public */,
       7,    0,   79,    2, 0x06 /* Public */,
       8,    1,   80,    2, 0x06 /* Public */,
      10,    0,   83,    2, 0x06 /* Public */,
      11,    1,   84,    2, 0x06 /* Public */,
      13,    0,   87,    2, 0x06 /* Public */,
      14,    0,   88,    2, 0x06 /* Public */,

 // methods: name, argc, parameters, tag, flags
      15,    1,   89,    2, 0x02 /* Public */,
      18,    0,   92,    2, 0x02 /* Public */,
      19,    0,   93,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::QString,    3,    5,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Bool, 0x80000000 | 16,   17,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      20, QMetaType::QStringList, 0x00495003,

 // properties: notify_signal_id
       7,

       0        // eod
};

void Session::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Session *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->stageStarting((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->stageFailed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 2: _t->connectionStarted(); break;
        case 3: _t->displayLaunchError((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->quitStarting(); break;
        case 5: _t->sessionFinished((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->readyForDeletion(); break;
        case 7: _t->launchWarningsChanged(); break;
        case 8: { bool _r = _t->initialize((*reinterpret_cast< QQuickWindow*(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 9: _t->start(); break;
        case 10: _t->interrupt(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QQuickWindow* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Session::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Session::stageStarting)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Session::*)(QString , int , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Session::stageFailed)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Session::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Session::connectionStarted)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Session::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Session::displayLaunchError)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Session::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Session::quitStarting)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Session::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Session::sessionFinished)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (Session::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Session::readyForDeletion)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (Session::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Session::launchWarningsChanged)) {
                *result = 7;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<Session *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QStringList*>(_v) = _t->m_LaunchWarnings; break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<Session *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0:
            if (_t->m_LaunchWarnings != *reinterpret_cast< QStringList*>(_v)) {
                _t->m_LaunchWarnings = *reinterpret_cast< QStringList*>(_v);
                Q_EMIT _t->launchWarningsChanged();
            }
            break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject Session::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_Session.data,
    qt_meta_data_Session,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Session::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Session::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Session.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Session::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void Session::stageStarting(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Session::stageFailed(QString _t1, int _t2, QString _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Session::connectionStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void Session::displayLaunchError(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Session::quitStarting()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void Session::sessionFinished(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void Session::readyForDeletion()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void Session::launchWarningsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
