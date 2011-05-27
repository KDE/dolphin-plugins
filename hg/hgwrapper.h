#ifndef HGWRAPPER_H
#define HGWRAPPER_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QDebug>
#include <kfileitem.h>

class HgWrapper : public QProcess
{
    Q_OBJECT
public:
    HgWrapper(QObject *parent = 0);

    static HgWrapper* instance();
    static void freeInstance();

    bool isBusy() const;
    void executeCommand(const QString& hgCommand,
                        const QStringList& arguments);
    QString getBaseDir(const QString &directory);
    void addFiles(const KFileItemList& fileList);
    void removeFiles(const KFileItemList& fileList);

private slots:
    void slotOperationCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void slotOperationError();

private:
private:
    static HgWrapper *m_instance;
    static bool m_pendingOperation;

    QStringList m_arguments;
};

#endif // HGWRAPPER_H
