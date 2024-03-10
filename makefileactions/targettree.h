/*
 * SPDX-FileCopyrightText: 2022 Pablo Rauzy <r .at. uzy.me>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MAKEFILEACTIONS_TARGET_H
#define MAKEFILEACTIONS_TARGET_H

#include <QDebug>
#include <QList>
#include <QString>

class TargetTree
{
public:
    TargetTree()
        : m_isTarget(false){};

    TargetTree(const QString &prefix, bool isTarget)
        : m_prefix(prefix)
        , m_isTarget(isTarget){};

    bool isTarget() const
    {
        return m_isTarget;
    };

    QString prefix() const
    {
        return m_prefix;
    };

    QList<TargetTree> children() const
    {
        return m_children;
    };

    bool insert(const QString &prefix, bool isTarget)
    {
        if (!prefix.startsWith(m_prefix)) { // nothing to do here
            return false;
        }
        if (prefix == m_prefix) { // already there
            if (isTarget) {
                m_isTarget = true;
            }
            return true;
        }
        // can we go deeper?
        for (auto &child : m_children) {
            if (child.insert(prefix, isTarget)) {
                return true;
            }
        }
        // if we couldn't, insert here
        m_children.append(TargetTree(prefix, isTarget));
        return true;
    };

    static bool cmp(const TargetTree &a, const TargetTree &b)
    {
        return (a.m_children.isEmpty() && !b.m_children.isEmpty()) || (a.m_children.isEmpty() && (a.m_prefix < b.m_prefix)) || (a.m_prefix < b.m_prefix);
    };

    friend QDebug operator<<(QDebug dbg, const TargetTree &t)
    {
        static int indent = 0;
        dbg << QString(indent, QLatin1Char(' ')) << t.m_prefix << (t.m_isTarget ? "@\n" : "\n");
        indent += 2;
        for (const TargetTree &c : t.m_children) {
            dbg << c;
        }
        indent -= 2;
        return dbg;
    };

private:
    QString m_prefix;
    bool m_isTarget;
    QList<TargetTree> m_children;
};

#endif
