/****************************************************************************
** Meta object code from reading C++ file 'listapps.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../cli/listapps.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'listapps.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CliListApps__Launcher_t {
    QByteArrayData data[10];
    char stringdata0[126];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CliListApps__Launcher_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CliListApps__Launcher_t qt_meta_stringdata_CliListApps__Launcher = {
    {
QT_MOC_LITERAL(0, 0, 21), // "CliListApps::Launcher"
QT_MOC_LITERAL(1, 22, 15), // "onComputerFound"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 11), // "NvComputer*"
QT_MOC_LITERAL(4, 51, 8), // "computer"
QT_MOC_LITERAL(5, 60, 21), // "onComputerSeekTimeout"
QT_MOC_LITERAL(6, 82, 7), // "execute"
QT_MOC_LITERAL(7, 90, 16), // "ComputerManager*"
QT_MOC_LITERAL(8, 107, 7), // "manager"
QT_MOC_LITERAL(9, 115, 10) // "isExecuted"

    },
    "CliListApps::Launcher\0onComputerFound\0"
    "\0NvComputer*\0computer\0onComputerSeekTimeout\0"
    "execute\0ComputerManager*\0manager\0"
    "isExecuted"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CliListApps__Launcher[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x08 /* Private */,
       5,    0,   37,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
       6,    1,   38,    2, 0x02 /* Public */,
       9,    0,   41,    2, 0x02 /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Bool,

       0        // eod
};

void CliListApps::Launcher::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Launcher *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onComputerFound((*reinterpret_cast< NvComputer*(*)>(_a[1]))); break;
        case 1: _t->onComputerSeekTimeout(); break;
        case 2: _t->execute((*reinterpret_cast< ComputerManager*(*)>(_a[1]))); break;
        case 3: { bool _r = _t->isExecuted();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CliListApps::Launcher::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CliListApps__Launcher.data,
    qt_meta_data_CliListApps__Launcher,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CliListApps::Launcher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CliListApps::Launcher::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CliListApps__Launcher.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int CliListApps::Launcher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
