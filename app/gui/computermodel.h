#include "backend/computermanager.h"
#include "streaming/session.h"

#include <QAbstractListModel>
#include <QVariantMap>

class ComputerModel : public QAbstractListModel
{
    Q_OBJECT

    enum Roles
    {
        NameRole = Qt::UserRole,
        OnlineRole,
        PairedRole,
        BusyRole,
        WakeableRole,
        StatusUnknownRole,
        ServerSupportedRole,
        HestiaEnhancedRole,
        HestiaDisplayRecoveryRole,
        HestiaServerCommandsRole,
        HestiaPermissionSystemRole,
        HestiaClipboardSyncRole,
        HestiaReadinessWarningRole,
        HestiaReadinessDetailsRole,
        DetailsRole
    };

public:
    explicit ComputerModel(QObject* object = nullptr);

    // Must be called before any QAbstractListModel functions
    Q_INVOKABLE void initialize(ComputerManager* computerManager);

    QVariant data(const QModelIndex &index, int role) const override;

    int rowCount(const QModelIndex &parent) const override;

    virtual QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void deleteComputer(int computerIndex);

    Q_INVOKABLE QString generatePinString();

    Q_INVOKABLE void pairComputer(int computerIndex, QString pin);

    Q_INVOKABLE void pairComputerOtp(int computerIndex, QString otpPin, QString otpPassphrase);

    Q_INVOKABLE void testConnectionForComputer(int computerIndex);

    Q_INVOKABLE void wakeComputer(int computerIndex);

    Q_INVOKABLE void renameComputer(int computerIndex, QString name);

    Q_INVOKABLE QVariantMap getHestiaDisplayStatus(int computerIndex);

    Q_INVOKABLE bool recoverHestiaDisplay(int computerIndex);

    Q_INVOKABLE QVariantMap getHestiaClientPermissions(int computerIndex);

    Q_INVOKABLE QVariantMap getHestiaDiagnostics(int computerIndex);

    Q_INVOKABLE bool pasteHestiaClipboard(int computerIndex);

    Q_INVOKABLE bool sendHestiaClipboard(int computerIndex);

    Q_INVOKABLE QVariantList getHestiaServerCommands(int computerIndex);

    Q_INVOKABLE bool runHestiaServerCommand(int computerIndex, int commandId);

    Q_INVOKABLE Session* createSessionForCurrentGame(int computerIndex);

signals:
    void pairingCompleted(QVariant error);
    void connectionTestCompleted(int result, QString blockedPorts);

private slots:
    void handleComputerStateChanged(NvComputer* computer);

    void handlePairingCompleted(NvComputer* computer, QString error);

private:
    QVector<NvComputer*> m_Computers;
    ComputerManager* m_ComputerManager;
};
