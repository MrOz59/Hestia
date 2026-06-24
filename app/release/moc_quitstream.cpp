/****************************************************************************
** Meta object code from reading C++ file 'quitstream.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../cli/quitstream.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'quitstream.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CliQuitStream__Launcher_t {
    QByteArrayData data[16];
    char stringdata0[195];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CliQuitStream__Launcher_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CliQuitStream__Launcher_t qt_meta_stringdata_CliQuitStream__Launcher = {
    {
QT_MOC_LITERAL(0, 0, 23), // "CliQuitStream::Launcher"
QT_MOC_LITERAL(1, 24, 17), // "searchingComputer"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 11), // "quittingApp"
QT_MOC_LITERAL(4, 55, 6), // "failed"
QT_MOC_LITERAL(5, 62, 4), // "text"
QT_MOC_LITERAL(6, 67, 15), // "onComputerFound"
QT_MOC_LITERAL(7, 83, 11), // "NvComputer*"
QT_MOC_LITERAL(8, 95, 8), // "computer"
QT_MOC_LITERAL(9, 104, 21), // "onComputerSeekTimeout"
QT_MOC_LITERAL(10, 126, 18), // "onQuitAppCompleted"
QT_MOC_LITERAL(11, 145, 5), // "error"
QT_MOC_LITERAL(12, 151, 7), // "execute"
QT_MOC_LITERAL(13, 159, 16), // "ComputerManager*"
QT_MOC_LITERAL(14, 176, 7), // "manager"
QT_MOC_LITERAL(15, 184, 10) // "isExecuted"

    },
    "CliQuitStream::Launcher\0searchingComputer\0"
    "\0quittingApp\0failed\0text\0onComputerFound\0"
    "NvComputer*\0computer\0onComputerSeekTimeout\0"
    "onQuitAppCompleted\0error\0execute\0"
    "ComputerManager*\0manager\0isExecuted"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CliQuitStream__Launcher[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,
       3,    0,   55,    2, 0x06 /* Public */,
       4,    1,   56,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   59,    2, 0x08 /* Private */,
       9,    0,   62,    2, 0x08 /* Private */,
      10,    1,   63,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      12,    1,   66,    2, 0x02 /* Public */,
      15,    0,   69,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QVariant,   11,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Bool,

       0        // eod
};

void CliQuitStream::Launcher::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Launcher *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->searchingComputer(); break;
        case 1: _t->quittingApp(); break;
        case 2: _t->failed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->onComputerFound((*reinterpret_cast< NvComputer*(*)>(_a[1]))); break;
        case 4: _t->onComputerSeekTimeout(); break;
        case 5: _t->onQuitAppCompleted((*reinterpret_cast< QVariant(*)>(_a[1]))); break;
        case 6: _t->execute((*reinterpret_cast< ComputerManager*(*)>(_a[1]))); break;
        case 7: { bool _r = _t->isExecuted();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Launcher::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Launcher::searchingComputer)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Launcher::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Launcher::quittingApp)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Launcher::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&Launcher::failed)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CliQuitStream::Launcher::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CliQuitStream__Launcher.data,
    qt_meta_data_CliQuitStream__Launcher,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CliQuitStream::Launcher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CliQuitStream::Launcher::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CliQuitStream__Launcher.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CliQuitStream::Launcher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void CliQuitStream::Launcher::searchingComputer()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CliQuitStream::Launcher::quittingApp()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void CliQuitStream::Launcher::failed(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
