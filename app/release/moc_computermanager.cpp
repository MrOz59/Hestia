/****************************************************************************
** Meta object code from reading C++ file 'computermanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../backend/computermanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'computermanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DelayedFlushThread_t {
    QByteArrayData data[1];
    char stringdata0[19];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DelayedFlushThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DelayedFlushThread_t qt_meta_stringdata_DelayedFlushThread = {
    {
QT_MOC_LITERAL(0, 0, 18) // "DelayedFlushThread"

    },
    "DelayedFlushThread"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DelayedFlushThread[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void DelayedFlushThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject DelayedFlushThread::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_DelayedFlushThread.data,
    qt_meta_data_DelayedFlushThread,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DelayedFlushThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DelayedFlushThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DelayedFlushThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int DelayedFlushThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_MdnsPendingComputer_t {
    QByteArrayData data[9];
    char stringdata0[143];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MdnsPendingComputer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MdnsPendingComputer_t qt_meta_stringdata_MdnsPendingComputer = {
    {
QT_MOC_LITERAL(0, 0, 19), // "MdnsPendingComputer"
QT_MOC_LITERAL(1, 20, 12), // "resolvedHost"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 20), // "MdnsPendingComputer*"
QT_MOC_LITERAL(4, 55, 22), // "QVector<QHostAddress>&"
QT_MOC_LITERAL(5, 78, 21), // "handleResolvedTimeout"
QT_MOC_LITERAL(6, 100, 21), // "handleResolvedAddress"
QT_MOC_LITERAL(7, 122, 12), // "QHostAddress"
QT_MOC_LITERAL(8, 135, 7) // "address"

    },
    "MdnsPendingComputer\0resolvedHost\0\0"
    "MdnsPendingComputer*\0QVector<QHostAddress>&\0"
    "handleResolvedTimeout\0handleResolvedAddress\0"
    "QHostAddress\0address"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MdnsPendingComputer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   34,    2, 0x08 /* Private */,
       6,    1,   35,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 4,    2,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void MdnsPendingComputer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MdnsPendingComputer *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->resolvedHost((*reinterpret_cast< MdnsPendingComputer*(*)>(_a[1])),(*reinterpret_cast< QVector<QHostAddress>(*)>(_a[2]))); break;
        case 1: _t->handleResolvedTimeout(); break;
        case 2: _t->handleResolvedAddress((*reinterpret_cast< const QHostAddress(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< MdnsPendingComputer* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MdnsPendingComputer::*)(MdnsPendingComputer * , QVector<QHostAddress> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MdnsPendingComputer::resolvedHost)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MdnsPendingComputer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_MdnsPendingComputer.data,
    qt_meta_data_MdnsPendingComputer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MdnsPendingComputer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MdnsPendingComputer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MdnsPendingComputer.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MdnsPendingComputer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void MdnsPendingComputer::resolvedHost(MdnsPendingComputer * _t1, QVector<QHostAddress> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_ComputerManager_t {
    QByteArrayData data[21];
    char stringdata0[331];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ComputerManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ComputerManager_t qt_meta_stringdata_ComputerManager = {
    {
QT_MOC_LITERAL(0, 0, 15), // "ComputerManager"
QT_MOC_LITERAL(1, 16, 20), // "computerStateChanged"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 11), // "NvComputer*"
QT_MOC_LITERAL(4, 50, 8), // "computer"
QT_MOC_LITERAL(5, 59, 16), // "pairingCompleted"
QT_MOC_LITERAL(6, 76, 5), // "error"
QT_MOC_LITERAL(7, 82, 20), // "computerAddCompleted"
QT_MOC_LITERAL(8, 103, 7), // "success"
QT_MOC_LITERAL(9, 111, 20), // "detectedPortBlocking"
QT_MOC_LITERAL(10, 132, 16), // "quitAppCompleted"
QT_MOC_LITERAL(11, 149, 17), // "handleAboutToQuit"
QT_MOC_LITERAL(12, 167, 26), // "handleComputerStateChanged"
QT_MOC_LITERAL(13, 194, 25), // "handleMdnsServiceResolved"
QT_MOC_LITERAL(14, 220, 20), // "MdnsPendingComputer*"
QT_MOC_LITERAL(15, 241, 22), // "QVector<QHostAddress>&"
QT_MOC_LITERAL(16, 264, 9), // "addresses"
QT_MOC_LITERAL(17, 274, 12), // "startPolling"
QT_MOC_LITERAL(18, 287, 16), // "stopPollingAsync"
QT_MOC_LITERAL(19, 304, 18), // "addNewHostManually"
QT_MOC_LITERAL(20, 323, 7) // "address"

    },
    "ComputerManager\0computerStateChanged\0"
    "\0NvComputer*\0computer\0pairingCompleted\0"
    "error\0computerAddCompleted\0success\0"
    "detectedPortBlocking\0quitAppCompleted\0"
    "handleAboutToQuit\0handleComputerStateChanged\0"
    "handleMdnsServiceResolved\0"
    "MdnsPendingComputer*\0QVector<QHostAddress>&\0"
    "addresses\0startPolling\0stopPollingAsync\0"
    "addNewHostManually\0address"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ComputerManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x06 /* Public */,
       5,    2,   67,    2, 0x06 /* Public */,
       7,    2,   72,    2, 0x06 /* Public */,
      10,    1,   77,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   80,    2, 0x08 /* Private */,
      12,    1,   81,    2, 0x08 /* Private */,
      13,    2,   84,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      17,    0,   89,    2, 0x02 /* Public */,
      18,    0,   90,    2, 0x02 /* Public */,
      19,    1,   91,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QString,    4,    6,
    QMetaType::Void, QMetaType::QVariant, QMetaType::QVariant,    8,    9,
    QMetaType::Void, QMetaType::QVariant,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 14, 0x80000000 | 15,    4,   16,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   20,

       0        // eod
};

void ComputerManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ComputerManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->computerStateChanged((*reinterpret_cast< NvComputer*(*)>(_a[1]))); break;
        case 1: _t->pairingCompleted((*reinterpret_cast< NvComputer*(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 2: _t->computerAddCompleted((*reinterpret_cast< QVariant(*)>(_a[1])),(*reinterpret_cast< QVariant(*)>(_a[2]))); break;
        case 3: _t->quitAppCompleted((*reinterpret_cast< QVariant(*)>(_a[1]))); break;
        case 4: _t->handleAboutToQuit(); break;
        case 5: _t->handleComputerStateChanged((*reinterpret_cast< NvComputer*(*)>(_a[1]))); break;
        case 6: _t->handleMdnsServiceResolved((*reinterpret_cast< MdnsPendingComputer*(*)>(_a[1])),(*reinterpret_cast< QVector<QHostAddress>(*)>(_a[2]))); break;
        case 7: _t->startPolling(); break;
        case 8: _t->stopPollingAsync(); break;
        case 9: _t->addNewHostManually((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< MdnsPendingComputer* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ComputerManager::*)(NvComputer * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ComputerManager::computerStateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ComputerManager::*)(NvComputer * , QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ComputerManager::pairingCompleted)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ComputerManager::*)(QVariant , QVariant );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ComputerManager::computerAddCompleted)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ComputerManager::*)(QVariant );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ComputerManager::quitAppCompleted)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ComputerManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ComputerManager.data,
    qt_meta_data_ComputerManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ComputerManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ComputerManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ComputerManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ComputerManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void ComputerManager::computerStateChanged(NvComputer * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ComputerManager::pairingCompleted(NvComputer * _t1, QString _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ComputerManager::computerAddCompleted(QVariant _t1, QVariant _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void ComputerManager::quitAppCompleted(QVariant _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
