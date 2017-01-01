/***********************************************************************************
**    Copyright (C) 2016  Petref Saraci
**    http://risip.io
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You have received a copy of the GNU General Public License
**    along with this program. See LICENSE.GPLv3
**    A copy of the license can be found also here <http://www.gnu.org/licenses/>.
**
************************************************************************************/
#ifndef RISIPCONTACTMANAGER_H
#define RISIPCONTACTMANAGER_H

#include "risipsdkglobal.h"
#include <QObject>
#include <QQmlListProperty>

class RisipAccount;
class RisipBuddiesModel;
class RisipContactHistoryModel;
class RisipiOSContactAcessManager;
class RisipPhoneContactsModel;
class RisipPhoneContact;
class RisipPhoneNumber;

class RISIP_SDK_EXPORT RisipContactManager : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(RisipAccount * activeAccount READ activeAccount WRITE setActiveAccount NOTIFY activeAccountChanged)
    Q_PROPERTY(RisipBuddiesModel * activeBuddiesModel READ activeBuddiesModel WRITE setActiveBuddiesModel NOTIFY activeBuddiesModelChanged)
    Q_PROPERTY(RisipContactHistoryModel * activeContactHistory READ activeContactHistory WRITE setActiveContactHistory NOTIFY activeContactHistoryChanged)
    Q_PROPERTY(RisipPhoneContact * activePhoneContact READ activePhoneContact WRITE setActivePhoneContact NOTIFY activePhoneContactChanged)
    Q_PROPERTY(RisipPhoneContactsModel * phoneContactsModel READ phoneContactsModel NOTIFY phoneContactsModelChanged)
    Q_PROPERTY(QQmlListProperty<RisipBuddiesModel> buddyModels READ buddyModels NOTIFY buddyModelsChanged)
    Q_PROPERTY(QQmlListProperty<RisipContactHistoryModel> contactHistoryModels READ contactHistoryModels NOTIFY contactHistoryModelsChanged)
    Q_PROPERTY(QQmlListProperty<RisipPhoneContact> phoneContacts READ phoneContacts)

    static RisipContactManager *instance();
    ~RisipContactManager();

    RisipAccount *activeAccount() const;
    void setActiveAccount(RisipAccount *activeAccount);

    RisipPhoneContact *activePhoneContact() const;
    void setActivePhoneContact(RisipPhoneContact *phoneContact);
    Q_INVOKABLE void setActivePhoneContact(const QString &contactName);

    RisipBuddiesModel *activeBuddiesModel() const;
    RisipContactHistoryModel *activeContactHistory() const;
    RisipPhoneContactsModel *phoneContactsModel() const;

    QQmlListProperty<RisipBuddiesModel> buddyModels();
    QQmlListProperty<RisipContactHistoryModel> contactHistoryModels();

    QQmlListProperty<RisipPhoneContact> phoneContacts();
    QList<RisipPhoneContact *> phoneContactList() const;

    void createModelsForAccount(RisipAccount *account);
    void removeModelsForAccount(const RisipAccount *account);

    RisipBuddiesModel *buddyModelForAccount(const QString &account) const;
    RisipContactHistoryModel *contactHistoryModelForAccount(const QString &account) const;

    Q_INVOKABLE void fetchPhoneContacts();
    Q_INVOKABLE RisipPhoneContact *contactForName(const QString &name);
    Q_INVOKABLE RisipPhoneContact *contactForIndex(int index);
    Q_INVOKABLE RisipPhoneNumber *phoneNumberForNumber(const QString &number);

Q_SIGNALS:
    void activeAccountChanged(RisipAccount *activeAccount);
    void activeBuddiesModelChanged(RisipBuddiesModel *model);
    void activeContactHistoryChanged(RisipContactHistoryModel *model);
    void activePhoneContactChanged(RisipPhoneContact *contact);
    void buddyModelsChanged();
    void contactHistoryModelsChanged();
    void phoneContactsModelChanged(RisipPhoneContactsModel *model);

private Q_SLOTS:
    void phoneContactDiscovered(RisipPhoneContact *contact);

private:
    explicit RisipContactManager(QObject *parent = 0);
    static RisipContactManager *m_instance;
    void setActiveBuddiesModel(RisipBuddiesModel *model);
    void setActiveContactHistory(RisipContactHistoryModel *history);

    class Private;
    Private *m_data;
};

#endif // RISIPCONTACTMANAGER_H