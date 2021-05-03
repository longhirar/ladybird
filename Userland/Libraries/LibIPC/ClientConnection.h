/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibIPC/Connection.h>

namespace IPC {

template<typename T, class... Args>
NonnullRefPtr<T> new_client_connection(Args&&... args)
{
    return T::construct(forward<Args>(args)...) /* arghs */;
}

template<typename ClientEndpoint, typename ServerEndpoint>
class ClientConnection : public Connection<ServerEndpoint, ClientEndpoint>, public ServerEndpoint::Stub {
public:
    using ClientProxy = typename ClientEndpoint::Proxy;
    using ServerStub = typename ServerEndpoint::Stub;

    ClientConnection(ServerStub& stub, NonnullRefPtr<Core::LocalSocket> socket, int client_id)
        : IPC::Connection<ServerEndpoint, ClientEndpoint>(stub, move(socket))
        , m_client_id(client_id)
    {
        VERIFY(this->socket().is_connected());
        this->socket().on_ready_to_read = [this] { this->drain_messages_from_peer(); };
    }

    virtual ~ClientConnection() override
    {
    }

    void did_misbehave()
    {
        dbgln("{} (id={}) misbehaved, disconnecting.", *this, m_client_id);
        this->shutdown();
    }

    void did_misbehave(const char* message)
    {
        dbgln("{} (id={}) misbehaved ({}), disconnecting.", *this, m_client_id, message);
        this->shutdown();
    }

    int client_id() const { return m_client_id; }

    virtual void die() = 0;

private:
    int m_client_id { -1 };
};

}

template<>
template<typename ClientEndpoint, typename ServerEndpoint>
struct AK::Formatter<IPC::ClientConnection<ClientEndpoint, ServerEndpoint>> : Formatter<Core::Object> {
};
