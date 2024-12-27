// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Applications.h"
#include "AppLauncher.h"

#include <memory>
#include "IAppLauncherClient.h"

namespace SscfTs1
{
	class ClientNode;
	class EventLoop;
	class EventHandler;
}

namespace CommonPlatform { namespace AppLauncher
{
	class AppLauncherClient:public IAppLauncherClient
	{
	public:
		AppLauncherClient(Applications source, bool subscribeAppExitEvents);
		~AppLauncherClient();

		bool WaitForConnection(int timeoutMillis);
		void DisconnectAsync();

		void Start(Applications app);
		void Stop(Applications app);

		void WarmRestart() const override;
		void ShutdownSystem() const override;
		void RebootSystem() const override;

        std::function<void(Applications,int)> EventAppExited;

	private:
		void ConnectAsync();
		void InternalDisconnect();

		void OnConnected();
		void OnDisconnected();
		void OnAppExit(Applications app, int exitCode);

		bool CreateProxies();

		/* Helper function to execute \p proxyFun on a connected IAppLauncher proxy. */
		template <typename ProxyFun>
		void WithProxy(ProxyFun&& proxyFun) const;

	    Applications								m_source;
		bool										m_subscribeAppExitEvents;
		std::unique_ptr<SscfTs1::EventLoop>			m_eventLoop;
		void*										m_connected; // Windows Event Object

		std::unique_ptr<SscfTs1::ClientNode>		m_client;
		std::unique_ptr<SscfTs1::EventHandler>		m_eventHandler;
		std::unique_ptr<Native::IAppLauncher>		m_appLauncher;
		std::unique_ptr<Native::IAppExitNotifier>	m_appExitNotifier;
	};
}}
