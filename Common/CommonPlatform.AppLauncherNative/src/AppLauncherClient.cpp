// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "AppLauncherClient.h"

#include <sscfts1.h>
#include <windows.h>
#include <assert.h>

#include "AppLauncherConstants.h"
#include "AppLauncher.h"
#include "Log.h"

namespace CommonPlatform { namespace AppLauncher
{
	AppLauncherClient::AppLauncherClient(Applications source, bool subscribeAppExitEvents) :
		m_source(source),
		m_subscribeAppExitEvents(subscribeAppExitEvents),
		m_eventLoop(new SscfTs1::EventLoop("AppLauncherClient_" + std::to_string(static_cast<int>(source)))),
		m_connected(CreateEvent(NULL, TRUE, FALSE, NULL))
	{
		m_eventLoop->start();
		ConnectAsync();
	}

	AppLauncherClient::~AppLauncherClient()
	{
		m_eventLoop->send([this]{ InternalDisconnect(); });
		if (m_eventLoop) m_eventLoop.reset();
		if (m_connected != NULL) CloseHandle(m_connected);
	}

	/** Waits until a connection has been established by the Sscf client, or until the timeout expires.
	 *  Returns true if there is an active connection before the timeout expires.
	 */
	bool AppLauncherClient::WaitForConnection(int timeoutMillis) 
	{
		return WaitForSingleObject(m_connected, timeoutMillis) == WAIT_OBJECT_0;
	}

	void AppLauncherClient::ConnectAsync()
	{
		m_eventLoop->send([this]
		{
			m_client.reset(new SscfTs1::ClientNode(*m_eventLoop, LoaderServerHost, LoaderServerUrn));
			m_eventHandler.reset(new SscfTs1::EventHandler());
			m_eventHandler->subscribe(m_client->connected, [this]{OnConnected();});
			m_eventHandler->subscribe(m_client->disconnected, [this]{OnDisconnected();});
			m_client->connectAsync();
		});
	}

	void AppLauncherClient::DisconnectAsync()
	{
		m_eventLoop->post([this]{ InternalDisconnect(); });
	}

	void AppLauncherClient::InternalDisconnect()
	{
		assert (m_eventLoop->checkThread());

		m_appLauncher.reset();
		m_appExitNotifier.reset();
		m_client->disconnect();
		m_eventHandler.reset();
		m_client.reset();
	}

	void AppLauncherClient::OnConnected()
	{
		assert (m_eventLoop->checkThread());

		assert (!m_appLauncher);
		assert (!m_appExitNotifier);

		if (CreateProxies())
		{
			SetEvent(m_connected);
		}
	}

	void AppLauncherClient::OnDisconnected()
	{
		assert (m_eventLoop->checkThread());

		m_appLauncher.reset();
		m_appExitNotifier.reset();
		m_client->connectAsync();

		ResetEvent(m_connected);
	}

	bool AppLauncherClient::CreateProxies()
	{
		assert (m_eventLoop->checkThread());

		try
		{
			m_appLauncher.reset(new Native::IAppLauncherProxy(*m_client));
			if (m_subscribeAppExitEvents)
			{
				m_appExitNotifier.reset(new Native::IAppExitNotifierProxy(*m_client));
				m_eventHandler->subscribe(m_appExitNotifier->EventAppExit, [this](Applications app, int exitCode)
				{
					OnAppExit(app, exitCode);
				});
			}
			return true;
		}
		catch (const SscfTs1::CommunicationException& ex)
		{
			auto &log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
			log.DeveloperInfo(L"AuditTrailClient::TryCreateProxy: Caught SscfTs1::CommunicationException --> %s", ex.what());

			m_appLauncher.reset();
			m_appExitNotifier.reset();
		}
		return false;
	}

    void AppLauncherClient::OnAppExit( Applications app, int exitCode )
    {
        if (EventAppExited) EventAppExited(app, exitCode);
    }

	template <typename ProxyFun>
	void AppLauncherClient::WithProxy(ProxyFun&& proxyFun) const
	{
		m_eventLoop->send([this,proxyFun]
		{
			if (!m_appLauncher)
			{
				auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
				log.DeveloperInfo(L"AppLauncherClient::WithProxy: client not connected");
			}
			else try
			{
				proxyFun(*m_appLauncher);
			}
			catch (const SscfTs1::CommunicationException&)
			{
				auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);
				log.DeveloperInfo(L"AppLauncherClient::WithProxy: Caught SscfTs1::CommunicationException");
			}
		});
	}

	void AppLauncherClient::Start(Applications app)
	{
		WithProxy([this,app](Native::IAppLauncher& proxy)
		{
			proxy.Start(app, m_source);
			// Workaround for PSC launch while an IST key is active, otherwise the OSK will not start during the IST login dialog
			if (app == Applications::FieldService)
			{
				proxy.Start(Applications::OnScreenKeyboard, Applications::FieldService);
			}
		});
	}

	void AppLauncherClient::Stop(Applications app)
	{
		WithProxy([this,app](Native::IAppLauncher& proxy)
		{
			proxy.Stop(app);
		});
	}

	void AppLauncherClient::WarmRestart() const
	{
		WithProxy([this](Native::IAppLauncher& proxy)
		{
			proxy.WarmRestart(m_source);
		});
	}

	void AppLauncherClient::ShutdownSystem() const
	{
		WithProxy([this](Native::IAppLauncher& proxy)
		{
			proxy.Shutdown(m_source);
		});
	}

	
	void AppLauncherClient::RebootSystem() const
	{
		WithProxy([this](Native::IAppLauncher& proxy)
		{
			proxy.Reboot(m_source);
		});
	}
}}
