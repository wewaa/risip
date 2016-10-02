/***********************************************************************************
**    Copyright (C) 2016  Petref Saraci
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
**    A copy of the license is also here <http://www.gnu.org/licenses/>.
**
************************************************************************************/

#include "risipbuddy.h"
#include "risipaccount.h"
#include "risipmessage.h"

#include <QDebug>

PjsipBuddy::PjsipBuddy()
    :Buddy()
{
}

PjsipBuddy::~PjsipBuddy()
{
}

void PjsipBuddy::onBuddyState()
{
    if(m_risipBuddyInterface != NULL)
        m_risipBuddyInterface->presenceChanged(m_risipBuddyInterface->presence());
}

void PjsipBuddy::setRisipBuddyInterface(RisipBuddy *risipBuddy)
{
    m_risipBuddyInterface = risipBuddy;
}

RisipBuddy::RisipBuddy(QObject *parent)
    :QObject(parent)
{
    //always subscribe to the buddy presence
    m_buddyConfig.subscribe = true;

}

RisipBuddy::~RisipBuddy()
{
    //TODO deleting all pending messages ? or move them to another list?
    releaseFailedPendingMessages();
}

RisipAccount *RisipBuddy::account() const
{
    return m_account;
}

void RisipBuddy::setAccount(RisipAccount *acc)
{
    if(m_account != acc) {
        m_account = acc;
        emit accountChanged(m_account);
    }
}

QString RisipBuddy::uri() const
{
    return QString::fromStdString(m_buddyConfig.uri);
}

void RisipBuddy::setUri(QString contactUri)
{
    //FIXME too many conversions of strings here..
    if(QString::fromStdString(m_buddyConfig.uri) != contactUri) {
        m_buddyConfig.uri = contactUri.toStdString();
        emit uriChanged(contactUri);
    }
}

QQmlListProperty<RisipMessage> RisipBuddy::messagesDeliveryPending()
{
    return QQmlListProperty<RisipMessage> (this, m_messagesDeliveryPending);
}

QQmlListProperty<RisipMessage> RisipBuddy::messagesDeliveryFailed()
{
    return QQmlListProperty<RisipMessage> (this, m_messagesDeliveryFailed);
}

PjsipBuddy *RisipBuddy::pjsipBuddy() const
{
    return m_pjsipBuddy;
}

/**
 * @brief RisipBuddy::setPjsipBuddy
 * @param buddy internal pjsip buddy object
 *
 * Internal function. Do not use. deletes all existing pending and failed messages
 */
void RisipBuddy::setPjsipBuddy(PjsipBuddy *buddy)
{
    m_pjsipBuddy = buddy;
    m_pjsipBuddy->setRisipBuddyInterface(this);
    m_buddyConfig.uri = buddy->getInfo().uri;

    //delete any existing pending and failed messages
    releaseFailedPendingMessages();
}

/**
 * @brief RisipBuddy::updateMessageStatus
 * @param message risip message object that has a status update with statusCode
 * @param statusCode the new status of the message
 *
 * Internal function. Do not use.
 */
void RisipBuddy::updateMessageStatus(RisipMessage *message, int statusCode)
{
    if(m_messagesDeliveryPending.contains(message)) {
        switch (statusCode) {
        case PJSIP_SC_OK:
        case PJSIP_SC_ACCEPTED:
            message->setStatus(RisipMessage::Sent);
            qDebug()<<"Message delivered: " <<message->messageBody();
            m_messagesDeliveryPending.removeAll(message);
            emit messagesDeliveryPendingChanged();
            return;
        }

        message->setStatus(RisipMessage::Failed);
        m_messagesDeliveryPending.removeAll(message);
        m_messagesDeliveryFailed.append(message);

        emit messagesDeliveryFailedChanged();
        emit messagesDeliveryPendingChanged();
    }
}

void RisipBuddy::releaseFailedPendingMessages()
{
    while (!m_messagesDeliveryFailed.isEmpty())
        m_messagesDeliveryFailed.takeFirst()->deleteLater();

    while (!m_messagesDeliveryPending.isEmpty())
        m_messagesDeliveryPending.takeFirst()->deleteLater();
}

void RisipBuddy::addToList()
{
    //check if uri and account are set - buddy cannot be created without those properties
    if(uri().isEmpty()
            || m_account == NULL)
        return;

    if(m_pjsipBuddy != NULL) {
        if(m_account->findBuddy(uri()))
            return;
        else
            delete m_pjsipBuddy;
    }

    m_pjsipBuddy = new PjsipBuddy;
    m_pjsipBuddy->setRisipBuddyInterface(this);
    try {
        m_pjsipBuddy->create(*m_account->pjsipAccount(), m_buddyConfig);
    } catch (Error &err) {
        qDebug()<<"Error creating/adding this buddy: " <<uri() << QString::fromStdString(err.info(true));
    }

    if(m_account) {
        m_account->addBuddy(this);
    }

    releaseFailedPendingMessages();
}

/**
 * @brief RisipBuddy::release
 *
 * Removes this buddy from the list and destroys it. All pending and failed messages are deleted.
 */
void RisipBuddy::release()
{
    if(m_account)
        m_account->removeBuddy(this);

    if(m_pjsipBuddy != NULL)
        delete m_pjsipBuddy;

    releaseFailedPendingMessages();
}

RisipMessage *RisipBuddy::sendInstantMessage(QString message)
{
    if(m_account == NULL
            || m_pjsipBuddy == NULL)
        return new RisipMessage();

    RisipMessage *risipMessage = new RisipMessage;
    risipMessage->setBuddy(this);
    risipMessage->setMessageBody(message);
    risipMessage->setStatus(RisipMessage::Pending);
    risipMessage->setDirection(RisipMessage::Outgoing);

    qDebug()<<"Sending message: " <<message;

    try {
        m_pjsipBuddy->sendInstantMessage(risipMessage->messageParamForSend());
    } catch (Error &err) {
        qDebug()<<"Error sending instant message to : " << uri() <<QString::fromStdString(err.info(true));
    }

    m_messagesDeliveryPending.append(risipMessage);
    emit messagesDeliveryPendingChanged();

    return risipMessage;
}

void RisipBuddy::sendInstantMessage(RisipMessage *message)
{
    message->setBuddy(this);
    try {
        m_pjsipBuddy->sendInstantMessage(message->messageParamForSend());
    } catch (Error &err) {
        qDebug()<<"Error sending instant message to : " << uri() <<QString::fromStdString(err.info(true));
    }

    m_messagesDeliveryPending.append(message);
}

int RisipBuddy::presence() const
{
    if(m_pjsipBuddy == NULL)
        return Null;

    switch (m_pjsipBuddy->getInfo().presStatus.status) {
    case PJSUA_BUDDY_STATUS_ONLINE:
        return Online;
    case PJSUA_BUDDY_STATUS_OFFLINE:
        return Offline;
    case PJSUA_BUDDY_STATUS_UNKNOWN:
        return Unknown;
    }

    return Unknown;
}