/*
    SPDX-FileCopyrightText: 2011 Vishesh Yadav <vishesh3y@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HGCLONEDIALOG_H
#define HGCLONEDIALOG_H

#include "dialogbase.h"
#include <QProcess>
#include <QString>

class QLineEdit;
class KPushButton;
class KTextEdit;
class QStackedLayout;
class QCheckBox;

// TODO: Enable to enter username/passwords if not found in config as well
//   as override within dialog

/**
 * Implements dialog to clone repository.
 */
class HgCloneDialog : public DialogBase
{
    Q_OBJECT

public:
    explicit HgCloneDialog(const QString &directory, QWidget *parent = nullptr);
    void setWorkingDirectory(const QString &directory);

private Q_SLOTS:
    void saveGeometry();

    /**
     * Enables dialog's Ok button when user has entered some input in
     * source LineEdit
     */
    void slotUpdateOkButton();
    void slotBrowseDestClicked();
    void slotBrowseSourceClicked();
    void slotCloningStarted();
    void slotCloningFinished(int exitCode, QProcess::ExitStatus);

    /**
     * Show output of clone operation in TextEdit component.
     */
    void slotUpdateCloneOutput();

private:
    void done(int r) override;
    void browseDirectory(QLineEdit *dest);
    void appendOptionArguments(QStringList &args);

private:
    QLineEdit *m_source;
    QLineEdit *m_destination;
    KPushButton *m_browse_dest;
    KPushButton *m_browse_source;
    KTextEdit *m_outputEdit;
    QStackedLayout *m_stackLayout;

    bool m_cloned;
    bool m_terminated;
    QString m_workingDirectory;
    QProcess m_process;

    // option checkboxes
    QCheckBox *m_optNoUpdate;
    QCheckBox *m_optUsePull;
    QCheckBox *m_optUseUncmprdTrans;
    QCheckBox *m_optNoVerifyServCert;
};

#endif // HGCLONEDIALOG_H
