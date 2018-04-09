#pragma once

bool get_system_font_path(const std::string& name, std::string& path)
{
	//
	// This code is not as safe as it should be.
	// Assumptions we make:
	//  -> GetWindowsDirectoryA does not fail.
	//  -> The registry key exists.
	//  -> The subkeys are ordered alphabetically
	//  -> The subkeys name and data are no longer than 260 (MAX_PATH) chars.
	//

	char buffer[MAX_PATH];
	HKEY registryKey;

	GetWindowsDirectoryA(buffer, MAX_PATH);
	std::string fontsFolder = buffer + std::string("\\Fonts\\");

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &registryKey)) {
		return false;
	}

	uint32_t valueIndex = 0;
	char valueName[MAX_PATH];
	uint8_t valueData[MAX_PATH];
	std::wstring wsFontFile;

	do {
		uint32_t valueNameSize = MAX_PATH;
		uint32_t valueDataSize = MAX_PATH;
		uint32_t valueType;

		auto error = RegEnumValueA(
			registryKey,
			valueIndex,
			valueName,
			reinterpret_cast<DWORD*>(&valueNameSize),
			0,
			reinterpret_cast<DWORD*>(&valueType),
			valueData,
			reinterpret_cast<DWORD*>(&valueDataSize));

		valueIndex++;

		if (error == ERROR_NO_MORE_ITEMS) {
			RegCloseKey(registryKey);
			return false;
		}

		if (error || valueType != REG_SZ) {
			continue;
		}

		if (_strnicmp(name.data(), valueName, name.size()) == 0) {
			path = fontsFolder + std::string((char*)valueData, valueDataSize);
			RegCloseKey(registryKey);
			return true;
		}
	} while (true);

	return false;
}

namespace CMenu
{
	bool legitTab = true;
	bool visualsTab = false;
	bool radarTab = false;
	bool changerTab = false;
	bool colorsTab = false;
	bool miscTab = false;

	bool visEspTab = true;
	bool visChamsTab = false;
	bool visHandsTab = false;
	bool visMiscTab = false;

	int windowWidth = 800;
	int windowHeight = 525;
	int curWidth = 1;
	int curHeight = 1;
	int curX = 0;
	int curY = 0;
	int tabHeight = 35;

	LONGLONG LastFrameTime;
	float deltaTime;

	LONGLONG milliseconds_now() {
		static LARGE_INTEGER s_frequency;
		static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
		if (s_use_qpc) {
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			return (1000LL * now.QuadPart) / s_frequency.QuadPart;
		}
		else {
			return GetTickCount();
		}
	}

	float GetDeltaTime()
	{
		LONGLONG ms = milliseconds_now();
		float ret = ms - LastFrameTime;
		LastFrameTime = ms;
		return ret;
		//return 1000.0f / ImGui::GetIO().Framerate;
	}

	int CalcTabWidth(int tabs)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		return ((windowWidth - (style.WindowPadding.x * 2)) / tabs) - (style.ItemSpacing.x * tabs);
		//return (curWidth - ((style.WindowPadding.x) + (style.ItemSpacing.x * tabs))) / tabs;
		//return windowWidth / tabs;
	}

	void SelectTab(bool* tab)
	{
		legitTab = false;
		visualsTab = false;
		radarTab = false;
		changerTab = false;
		colorsTab = false;
		miscTab = false;

		*tab = true;
	}

	void SelectVisualsSubTab(bool* tab)
	{
		visEspTab = false;
		visChamsTab = false;
		visHandsTab = false;
		visMiscTab = false;

		*tab = true;
	}

	bool DrawTab(char* name, int tabWidth, int tabHeight, bool *boole, bool forceTabs = true)
	{
		bool ret = false;
		if (ImGui::Tab(name, ImVec2(tabWidth, tabHeight), *boole))
		{
			ret = true;
			*boole = true;
			if (forceTabs) SelectTab(boole);
		}

		//ImGui::SameLine(0);
		return ret;
	}

	bool DrawTab(char* name, int tabWidth, int tabHeight, bool boole, bool forceTabs = true)
	{
		return DrawTab(name, tabWidth, tabHeight, &boole, forceTabs);
	}

	void DrawTabs()
	{
		int w, h;
		Interface.Engine->GetScreenSize(w, h);

		if (ImGui::Begin(XorStr("TirepsCheats | Private Hack"), &G::MenuOpened, ImVec2(700, 380), 1.0F, ImGuiWindowFlags_ShowBorders))
		{
			//ImGui::NewLine(3);

			ImGuiContext* io = ImGui::GetCurrentContext();
			ImGuiStyle& style = ImGui::GetStyle();
			int tabWidth = CalcTabWidth(6) + 20;

			float lastSize = io->FontSize;
			io->FontSize = 20;


			ImGui::BeginGroup();  //������� ������ ��������a
			DrawTab(XorStr("Legit"), tabWidth, tabHeight, &legitTab);
			DrawTab(XorStr("Visuals"), tabWidth, tabHeight, &visualsTab);
			DrawTab(XorStr("Radar"), tabWidth, tabHeight, &radarTab);
			DrawTab(XorStr("Paints"), tabWidth, tabHeight, &changerTab);
			DrawTab(XorStr("Colors"), tabWidth, tabHeight, &colorsTab);
			DrawTab(XorStr("Misc"), tabWidth, tabHeight, &miscTab);
			io->FontSize = lastSize;

			ImGui::Text(XorStr("Config"));
			if (ImGui::Button(XorStr("Load"), ImVec2(120, 0))) Config::Read();
			if (ImGui::Button(XorStr("Save"), ImVec2(120, 0))) Config::Save();

			ImGui::EndGroup(); //���������� ������� � ������

			ImGui::SameLine(/*posX*/); //�������� ����� � �����

			ImGui::BeginGroup();  //�������  ����� ������ ��������

			//ImGui::EndGroup(); //���������� ������� � ������

		}
		//ImGui::End();
	}

	void DrawRadar()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 oldPadding = style.WindowPadding;
		float oldAlpha = style.Colors[ImGuiCol_WindowBg].w;
		style.WindowPadding = ImVec2(0, 0);
		style.Colors[ImGuiCol_WindowBg].w = (float)Options::Radar::Alpha / 255.0f;
		int specs = 0;
		std::string spect = "";
		if (Interface.Engine->IsInGame() && Interface.Engine->IsConnected())
		{
			int localIndex = Interface.Engine->GetLocalPlayer();
			CPlayer* pLocalEntity = Interface.EntityList->GetClientEntity<CPlayer>(localIndex);
			if (pLocalEntity)
			{
				for (int i = 0; i < Interface.Engine->GetMaxClients(); i++) {
					CPlayer* pBaseEntity = Interface.EntityList->GetClientEntity<CPlayer>(i);
					if (!pBaseEntity)
						continue;
					if (pBaseEntity->GetHealth() > 0)
						continue;
					if (pBaseEntity == pLocalEntity)
						continue;
					if (pBaseEntity->GetDormant())
						continue;
					if (pBaseEntity->GetObserverTarget() != pLocalEntity)
						continue;

					player_info_t pInfo;
					Interface.Engine->GetPlayerInfo(pBaseEntity->GetIndex(), &pInfo);
					if (pInfo.ishltv)
						continue;

					spect += pInfo.name;
					spect += "\n";
					specs++;
				}
			}
		}
		if (ImGui::Begin(XorStr("Radar"), &G::MenuOpened, ImVec2(200, 200), 0.4F) /*| ImGuiWindowFlags_NoTitleBar*/ )
		{
			ImVec2 siz = ImGui::GetWindowSize();
			ImVec2 pos = ImGui::GetWindowPos();

			//_drawList->AddRect(ImVec2(pos.x - 6, pos.y - 6), ImVec2(pos.x + siz.x + 6, pos.y + siz.y + 6), Color::Black().GetU32(), 0.0F, -1, 1.5f);
			//_drawList->AddRect(ImVec2(pos.x - 2, pos.y - 2), ImVec2(pos.x + siz.x + 2, pos.y + siz.y + 2), Color::Black().GetU32(), 0.0F, -1, 1);

			//drawList->AddRect(ImVec2(pos.x - 2, pos.y - 2), ImVec2(pos.x + siz.x + 2, pos.y + siz.y + 2), Color::Black().GetU32(), 0.0F, -1, 2);
			//_drawList->AddRect(ImVec2(pos.x - 2, pos.y - 2), ImVec2(pos.x + siz.x + 2, pos.y + siz.y + 2), Color::Orange().GetU32(), 0.0F, -1, 1.1f);

			if (G::NextResetRadar)
			{
				ImGui::SetWindowSize(ImVec2(200, 200));
				G::NextResetRadar = false;
			}

			ImDrawList* windowDrawList = ImGui::GetWindowDrawList();
			windowDrawList->AddLine(ImVec2(pos.x + (siz.x / 2), pos.y + 0), ImVec2(pos.x + (siz.x / 2), pos.y + siz.y), Color::White().GetU32(), 1.5f);
			windowDrawList->AddLine(ImVec2(pos.x + 0, pos.y + (siz.y / 2)), ImVec2(pos.x + siz.x, pos.y + (siz.y / 2)), Color::White().GetU32(), 1.5f);

			// Rendering players

			if (Interface.Engine->IsInGame() && Interface.Engine->IsConnected())
			{
				CPlayer* pLocalEntity = Interface.EntityList->GetClientEntity<CPlayer>(Interface.Engine->GetLocalPlayer());
				if (pLocalEntity)
				{
					Vector3 LocalPos = pLocalEntity->GetEyePosition();
					Vector3 ang;
					Interface.Engine->GetViewAngles(ang);
					for (int i = 0; i < Interface.Engine->GetMaxClients(); i++) {
						CPlayer* pBaseEntity = Interface.EntityList->GetClientEntity<CPlayer>(i);

						if (!pBaseEntity)
							continue;
						if (!pBaseEntity->IsValid())
							continue;
						if (pBaseEntity == pLocalEntity)
							continue;

						bool bIsEnemy = pLocalEntity->GetTeam() != pBaseEntity->GetTeam();
						bool isVisibled = G::VisibledPlayers[i]; /*U::IsVisible(LocalPos, pBaseEntity->GetEyePosition(), pLocalEntity, pBaseEntity, Options::Radar::SmokeCheck);*/

						if (Options::Radar::EnemyOnly && !bIsEnemy)
							continue;

						bool viewCheck = false;
						Vector2 EntityPos = U::RotatePoint(pBaseEntity->GetOrigin(), LocalPos, pos.x, pos.y, siz.x, siz.y, ang.y, Options::Radar::Zoom, &viewCheck);

						if (!viewCheck && Options::Radar::ViewCheck)
							isVisibled = false;
						//ImU32 clr = (bIsEnemy ? (isVisibled ? Color::LightGreen() : Color::Blue()) : Color::White()).GetU32();
						ImU32 clr = (bIsEnemy ? (isVisibled ? Color::White() : Color::Black()) : Color::Lightblue()).GetU32();

						if (Options::Radar::VisibleOnly && !isVisibled)
							continue;

						int s = 4;
						switch (Options::Radar::Type) // 0 - Box; 1 - Filled box; 2 - Circle; 3 - Filled circle;
						{
						case 0:
						{
							windowDrawList->AddRect(ImVec2(EntityPos.x - s, EntityPos.y - s),
								ImVec2(EntityPos.x + s, EntityPos.y + s),
								clr);
							break;
						}
						case 1:
						{
							windowDrawList->AddRectFilled(ImVec2(EntityPos.x - s, EntityPos.y - s),
								ImVec2(EntityPos.x + s, EntityPos.y + s),
								clr);
							break;
						}
						case 2:
						{
							windowDrawList->AddCircle(ImVec2(EntityPos.x, EntityPos.y), s, clr);
							break;
						}
						case 3:
						{
							windowDrawList->AddCircleFilled(ImVec2(EntityPos.x, EntityPos.y), s, clr);
							break;
						}
						default:
							break;
						}
					}
				}
			}
		}
		ImGui::End();
		style.WindowPadding = oldPadding;
		style.Colors[ImGuiCol_WindowBg].w = oldAlpha;
	}
	void DrawWatermark()
	{
		if (ImGui::Begin(XorStr("Watermark"), &G::MenuOpened, ImVec2(210, 27), 0.4F, ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
		{
			ImGuiContext* io = ImGui::GetCurrentContext();
			ImGuiStyle& style = ImGui::GetStyle();
			io->FontSize = 20;
			ImGui::Text("TirepsCheats| Private Hack");
		}
		ImGui::End();
	}
	void DrawSpectatorList()
	{
		int specs = 0;
		std::string spect = "";
		if (Interface.Engine->IsInGame() && Interface.Engine->IsConnected())
		{
			int localIndex = Interface.Engine->GetLocalPlayer();
			CPlayer* pLocalEntity = Interface.EntityList->GetClientEntity<CPlayer>(localIndex);
			if (pLocalEntity)
			{
				for (int i = 0; i < Interface.Engine->GetMaxClients(); i++) {
					CPlayer* pBaseEntity = Interface.EntityList->GetClientEntity<CPlayer>(i);
					if (!pBaseEntity)
						continue;
					if (pBaseEntity->GetHealth() > 0)
						continue;
					if (pBaseEntity == pLocalEntity)
						continue;
					if (pBaseEntity->GetDormant())
						continue;
					if (pBaseEntity->GetObserverTarget() != pLocalEntity)
						continue;

					player_info_t pInfo;
					Interface.Engine->GetPlayerInfo(pBaseEntity->GetIndex(), &pInfo);
					if (pInfo.ishltv)
						continue;

					spect += pInfo.name;
					spect += "\n";
					specs++;
				}
			}
		}
		if (ImGui::Begin(XorStr("Custom UI"), &G::MenuOpened, ImVec2(200, 225), 0.4F, ImGuiWindowFlags_ShowBorders))
		{
			if (specs > 0) spect += "\n";
			ImVec2 siz = ImGui::CalcTextSize(spect.c_str());
			ImGui::Text("Damage:", CEvents::hurtDamage); ImGui::SameLine(); ImGui::Text(XorStr("%d"), CEvents::hurtDamage);
			ImGui::Separator();
			ImGui::Text("Names spectators:");
			ImGui::Separator();
			ImGui::Text(spect.c_str());
		}
		ImGui::End();
	}

	// DDD

	void DrawLegitTab()
	{
		ImVec2 lPos = ImGui::GetCursorPos();
		ImGui::Checkbox(XorStr("Active"), &Options::Legitbot::Enabled);
		ImGui::Checkbox(XorStr("Friendly Fire"), &Options::Legitbot::Deathmatch);
		ImGui::Checkbox(XorStr("Smoke Check"), &Options::Legitbot::SmokeCheck);
		ImGui::Checkbox(XorStr("Auto Pistol"), &Options::Misc::AU);
		ImGui::Checkbox(XorStr("Auto Shoot"), &Options::Legitbot::AutoFire);
		ImGui::Text(XorStr("Smooth type"));
		ImGui::Combo(XorStr(""), &Options::Legitbot::AimType, "Old\0New\0Curved\0\0", -1, ImVec2(130, 0));
		ImGui::Checkbox(XorStr("Kill Delay"), &Options::Legitbot::KillDelay);
		ImGui::SliderFloat(XorStr("##0"), &Options::Legitbot::KillDelayTime, 0, 2, "%.2f", 1.0F, ImVec2(130, 0));

		int cw = U::SafeWeaponID();
		if (cw == 0)
			return;
		if (U::IsNonAimWeapon(cw))
			return;

		ImGui::Separator();
		ImGui::Checkbox(XorStr("Active"), &weapons[cw].Enabled);
		ImGui::Checkbox(XorStr("Closest Bone"), &weapons[cw].Nearest);
		if (weapons[cw].Nearest)
		{
			ImGui::Combo(XorStr("##1"), &weapons[cw].NearestType, "Low\0Medium\0Max\0\0", -1);
		}
		else
			ImGui::InputInt(XorStr("Hitbox"), &weapons[cw].Bone);
		ImGui::SliderFloat(XorStr("Field Of View"), &weapons[cw].Fov, 0, 180);
		ImGui::SliderFloat(XorStr("Smooth Factor"), &weapons[cw].Smooth, 0, 10);
		ImGui::NewLine();

		ImGui::Checkbox(XorStr("Fire delay"), &weapons[cw].FireDelayEnabled);
		ImGui::Checkbox(XorStr("Repeat fire dellay"), &weapons[cw].FireDelayRepeat);
		ImGui::SliderFloat(XorStr("##2"), &weapons[cw].FireDelay, 0, 2, "%.4f");

		ImGui::NewLine();

		ImGui::SliderInt(XorStr("Recoil Pitch Factor"), &weapons[cw].RcsX, 0, 200, "%.0f %");
		ImGui::SliderInt(XorStr("Recoil Yaw Factor"), &weapons[cw].RcsY, 0, 200, "%.0f %");

		ImGui::NewLine();

		ImGui::SliderInt(XorStr("First bullet"), &weapons[cw].StartBullet, 0, 30);
		ImGui::SliderInt(XorStr("Last bullet"), &weapons[cw].EndBullet, 0, 30);

		ImGui::NewLine();

		ImGui::Checkbox(XorStr("Perfect Silent"), &weapons[cw].pSilent);
		ImGui::SliderInt(XorStr("Perfect Silent Percentage"), &weapons[cw].pSilentPercentage, 0, 100);
		ImGui::SliderInt(XorStr("Silent Bullet"), &weapons[cw].pSilentBullet, 0, 30);
		ImGui::SliderFloat(XorStr("Perfect Silent Field Of View##0"), &weapons[cw].pSilentFov, 0, 180);
		ImGui::SliderFloat(XorStr("Perfect Silent Smooth Factor##0"), &weapons[cw].pSilentSmooth, 0, 10);
	}

	void DrawColorsTab()
	{
		static int selectedItem = 0;

		ImVec2 lastCursor = ImGui::GetCursorPos();

		ImGui::ListBoxHeader("##0", ImVec2(240, 520));
		for (int i = 0; i < G::ColorsForPicker.size(); i++)
		{
			bool selected = i == selectedItem;

			if (ImGui::Selectable(G::ColorsForPicker[i].Name, selected))
				selectedItem = i;
		}
		ImGui::ListBoxFooter();

		float nc = lastCursor.x + 260;
		ImGui::SetCursorPos(ImVec2(nc, lastCursor.y));

		ColorP col = G::ColorsForPicker[selectedItem];
		int r = (col.Ccolor[0] * 255.f);
		int g = (col.Ccolor[1] * 255.f);
		int b = (col.Ccolor[2] * 255.f);
		//int a = (*col.Ccolor[3] * 255.f);

		ImGui::NewLine(); ImGui::SetCursorPosX(nc);
		ImGui::SliderInt("##R", &r, 0, 255, "%.0f", ImVec4(0.40f, 0, 0, 0));
		ImGui::NewLine(); ImGui::SetCursorPosX(nc);
		ImGui::SliderInt("##G", &g, 0, 255, "%.0f", ImVec4(0, 0.40f, 0, 0));
		ImGui::NewLine(); ImGui::SetCursorPosX(nc);
		ImGui::SliderInt("##B", &b, 0, 255, "%.0f", ImVec4(0, 0, 0.40f, 0));
		//ImGui::NewLine(); ImGui::SetCursorPosX(nc);
		//ImGui::SliderInt("A", &a, 0, 255, "%.0f", ImVec4(0, 0, 0, 0));

		col.Ccolor[0] = r / 255.0f;
		col.Ccolor[1] = g / 255.0f;
		col.Ccolor[2] = b / 255.0f;
		//col.Ccolor[3] = a / 255.0f;

		ImGui::NewLine(); ImGui::SetCursorPosX(nc);
		ImVec2 curPos = ImGui::GetCursorPos();
		ImVec2 curWindowPos = ImGui::GetWindowPos();
		curPos.x += curWindowPos.x;
		curPos.y += curWindowPos.y;


		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(curPos.x + 255, curPos.y),
			ImVec2(curWindowPos.x + curWidth - 25, curPos.y + 200),
			ImGui::GetColorU32(ImVec4(col.Ccolor[0], col.Ccolor[1], col.Ccolor[2], 255)));

		ImGui::ColorPicker("##COLOR_PICKER", col.Ccolor);

		ImGui::NewLine(); ImGui::NewLine(); ImGui::SetCursorPosX(nc);
	}

	void DrawVisualsTab()
	{
		//ImGui::SameLine();
		//ImGui::GetCurrentWindow()->DC.CursorPos.y += 6;
		int tabWidth = CalcTabWidth(4);

		/*if (DrawTab(XorStr("ESP"), tabWidth, tabHeight, visEspTab, false)) SelectVisualsSubTab(&visEspTab);
		if (DrawTab(XorStr("Chams"), tabWidth, tabHeight, visChamsTab, false)) SelectVisualsSubTab(&visChamsTab);
		if (DrawTab(XorStr("Hands"), tabWidth, tabHeight, visHandsTab, false)) SelectVisualsSubTab(&visHandsTab);
		if (DrawTab(XorStr("Misc##0"), tabWidth, tabHeight, visMiscTab, false)) SelectVisualsSubTab(&visMiscTab);*/
		//ImGui::NewLine();

		ImVec2 siz = ImVec2(185, curHeight - ImGui::GetCursorPosY() - 40);
		ImVec2 csize = ImVec2(siz.x - 28, 0);
		ImVec2 asize = ImVec2(csize.x - 10, 0);

		ImGui::Columns(2,false);
		ImGui::BeginGroup();  //������� ������ ��������a

		ImGui::Checkbox(XorStr("Active"), &Options::Visuals::ESP::Enabled);
		ImGui::Checkbox(XorStr("Aimbot Fov"), &Options::Legitbot::DrawFov);
		ImGui::Text(XorStr("Style"));
		ImGui::Combo(XorStr("Box Style"), &Options::Visuals::ESP::Style, "Default\0Default outlined\0Corner\0Corner outlined\0\r3D\0\r3D filled\0\r3D filled outline\0\0", -1, csize);
		ImGui::Checkbox(XorStr("Enemy only"), &Options::Visuals::ESP::EnemyOnly);
		ImGui::Checkbox(XorStr("Visible only"), &Options::Visuals::ESP::VisibleOnly);
		ImGui::Checkbox(XorStr("Smoke check"), &Options::Visuals::ESP::SmokeCheck);
		ImGui::Checkbox(XorStr("Player Box"), &Options::Visuals::ESP::Box);
		ImGui::Checkbox(XorStr("Player Health bar"), &Options::Visuals::ESP::Health);
		ImGui::Checkbox(XorStr("Player Name"), &Options::Visuals::ESP::Name);
		ImGui::Checkbox(XorStr("Player Weapon"), &Options::Visuals::ESP::Weapon);
		ImGui::Checkbox(XorStr("Player Ammo"), &Options::Visuals::ESP::WeaponAmmo);
		ImGui::Text(XorStr("Colored Models"));
		ImGui::Text(XorStr("Colored Models Type"));
		ImGui::Combo(XorStr("0"), &Options::Visuals::Chams::Style, "Default\0Flat\0Textured\0\0\0", -1, csize);
		ImGui::Checkbox(XorStr("Rainbow"), &Options::Visuals::Chams::RainBow);
		if (Options::Visuals::Chams::RainBow)
		{
			ImGui::SameLine(); ImGui::Checkbox(XorStr("Visible"), &Options::Visuals::Chams::VisibleRainBow);
			ImGui::SameLine(); ImGui::Checkbox(XorStr("In Visible"), &Options::Visuals::Chams::InVisibleRainBow);
		}
		ImGui::Checkbox(XorStr("Enable Hands"), &Options::Visuals::Hands::Enabled);

		ImGui::EndGroup(); //���������� ������� � ������

		ImGui::SameLine(/*posX*/); //�������� ����� � �����

		ImGui::NextColumn();
		ImGui::BeginGroup();  //�������  ����� ������ ��������
		ImGui::Text(XorStr("Hands"));
		ImGui::Text(XorStr("Style"));
		ImGui::Combo(XorStr(""), &Options::Visuals::Hands::Style, "Disabled\0Wireframed\0Chams\0Wireframed Chams\0Rainbow Chams\0Rainbow Wireframe\0\0", -1, csize);
		ImGui::Text(XorStr("Misc"));
		ImGui::Checkbox(XorStr("Droped ESP"), &Options::Visuals::Misc::DropESP);
		ImGui::Checkbox(XorStr("Grenades ESP"), &Options::Visuals::Misc::Grenades);
		ImGui::Checkbox(XorStr("Bomb ESP"), &Options::Visuals::Misc::BombTimer);
		ImGui::Text(XorStr("Bomb ESP Style"));
		ImGui::Combo(XorStr("##0"), &Options::Visuals::Misc::BombTimerType, "Bomb Place\0Timer\0Bomb Place & Timer\0\0", -1, csize);
		ImGui::Checkbox(XorStr("No Flash"), &Options::Misc::NoFlash);
		ImGui::SliderFloat(XorStr("##1"), &Options::Misc::NoFlashAlpha, 0, 255, "%.1f", 1.0F, asize);
		ImGui::Checkbox(XorStr("View Fov Changer"), &Options::Visuals::Misc::FovChanger);
		ImGui::SliderFloat(XorStr("##2"), &Options::Visuals::Misc::FovChangerValue, 70, 160, "%.1f", 1.0F, asize);
		ImGui::Checkbox(XorStr("View Model Changer"), &Options::Visuals::Misc::ViewmodelChanger);
		ImGui::SliderFloat(XorStr("##3"), &Options::Visuals::Misc::ViewmodelChangerValue, 70, 160, "%.1f", 1.0F, asize);
		ImGui::Checkbox(XorStr("Dlight"), &Options::Visuals::ESP::Lights);
		ImGui::EndGroup(); //���������� ������� � ������
	}
	void DrawRadarTab()
	{
		ImGui::Checkbox(XorStr("Active"), &Options::Radar::Enabled);
		ImGui::Text(XorStr("Dot Type"));
		ImGui::Combo(XorStr("Dot Type"), &Options::Radar::Type, "Box\0Filled Box\0Circle\0Filled Circle\0\0", -1);
		ImGui::Checkbox(XorStr("Enemy only"), &Options::Radar::EnemyOnly);
		ImGui::Checkbox(XorStr("Visible only"), &Options::Radar::VisibleOnly);
		ImGui::Checkbox(XorStr("Smoke check"), &Options::Radar::SmokeCheck);
		ImGui::Checkbox(XorStr("View check"), &Options::Radar::ViewCheck);
		ImGui::Text(XorStr("Radar Alpha")); ImGui::SameLine();
		ImGui::SliderInt(XorStr("Radar Alpha##0"), &Options::Radar::Alpha, 0, 255);
		ImGui::Text(XorStr("Radar Zoom")); ImGui::SameLine();
		ImGui::SliderFloat(XorStr("Radar Zoom##0"), &Options::Radar::Zoom, 0, 4);
		ImGui::Text(XorStr("Radar Ttyle"));
		ImGui::Combo(XorStr("Radar Type"), &Options::Radar::Style, "Window\0In-Game\0\0");
		if (ImGui::Button(XorStr("Reset Radar Size"), ImVec2(120, 0))) G::NextResetRadar = true;
	}



	bool AK47;
	bool M4;
	bool PLAYERT;
	bool PLAYERCT;
	bool KNIFECT;
	bool KNIFET;
	bool Ismodels;

	void DrawChangerTab()
	{
		ImVec2 siz = ImVec2(185, curHeight - ImGui::GetCursorPosY() - 40);
		ImVec2 csize = ImVec2(siz.x - 28, 0);
		ImVec2 asize = ImVec2(csize.x - 10, 0);

		ImVec2 lPos = ImGui::GetCursorPos();
		ImGui::Combo(XorStr("Sky Changer"), &Options::Visuals::Misc::CustomSky, "None\0Vertigo\0Night\0Vertigo Blue\0Vietnam\0Italy\0Night 2\0Tibet\0Jungle\0Amethyst\0\0", -1, csize);
		if (ImGui::Checkbox(XorStr("Skins Changer##0"), &Options::SkinChanger::EnabledSkin)) U::FullUpdate();

		if (ImGui::Checkbox(XorStr("Knife Changer"), &Options::SkinChanger::EnabledKnife)) U::FullUpdate();
		if (Options::SkinChanger::EnabledKnife)
			if (ImGui::Combo(XorStr("Knife Model##0"), &Options::SkinChanger::Knife, "Bayonet\0Flip\0Gut\0Karambit\0M9Bayonet\0Huntsman\0Falchion\0Bowie\0Butterfly\0Daggers\0\0", -1, ImVec2(130, 0)))
				U::FullUpdate();

		if (ImGui::Checkbox(XorStr("Glove Changer"), &Options::SkinChanger::EnabledGlove)) U::FullUpdate();
		if (Options::SkinChanger::EnabledGlove)
		{
			if (ImGui::Combo(XorStr("Glove Model##1"), &Options::SkinChanger::Glove, "Bloodhound\0Sport\0Driver\0Wraps\0Moto\0Specialist\0\0", -1, ImVec2(130, 0)))
				U::FullUpdate();

			const char* gstr;
			if (Options::SkinChanger::Glove == 0)
			{
				gstr = "Charred\0\rSnakebite\0\rBronzed\0\rGuerilla\0\0";
			}
			else if (Options::SkinChanger::Glove == 1)
			{
				gstr = "Hedge Maze\0\rPandoras Box\0\rSuperconductor\0\rArid\0\0";
			}
			else if (Options::SkinChanger::Glove == 2)
			{
				gstr = "Lunar Weave\0\rConvoy\0\rCrimson Weave\0\rDiamondback\0\0";
			}
			else if (Options::SkinChanger::Glove == 3)
			{
				gstr = "Leather\0\rSpruce DDPAT\0\rSlaughter\0\rBadlands\0\0";
			}
			else if (Options::SkinChanger::Glove == 4)
			{
				gstr = "Eclipse\0\rSpearmint\0\rBoom!\0\rCool Mint\0\0";
			}
			else if (Options::SkinChanger::Glove == 5)
			{
				gstr = "Forest DDPAT\0\rCrimson Kimono\0\rEmerald Web\0\rFoundation\0\0";
			}
			else
				gstr = "";

			if (ImGui::Combo(XorStr("Glove Skin##2"), &Options::SkinChanger::GloveSkin, gstr, -1, ImVec2(130, 0)))
				U::FullUpdate();

			ImGui::Checkbox(XorStr("Custom Model Changer"), &Ismodels);

			if (Ismodels)
			{
				ImGui::PushItemWidth(ImGui::GetWindowWidth() - 30);
				ImGui::Text("Counter-Terorist Model");
				ImGui::Checkbox(XorStr("Counter-Terorist Model"), &PLAYERCT);
				if (PLAYERCT)
					ImGui::InputText(XorStr("##Name"), Options::SkinChanger::buf1, 128);

				ImGui::Text("Terorist Model");
				ImGui::Checkbox(XorStr("Terorist Model"), &PLAYERT);
				if (PLAYERT)
					ImGui::InputText(XorStr("##Name1"), Options::SkinChanger::buf2, 128);

				ImGui::Text("AK-47 Model");
				ImGui::Checkbox(XorStr("AK-47 Model"), &AK47);
				if (AK47)
					ImGui::InputText(XorStr("##Name2"), Options::SkinChanger::buf3, 128);

				ImGui::Text("M4A4 Model");
				ImGui::Checkbox(XorStr("M4A4 Model"), &M4);
				if (M4)
					ImGui::InputText(XorStr("##Name3"), Options::SkinChanger::buf4, 128);

				ImGui::Text("Terorist Knife Model");
				ImGui::Checkbox(XorStr("Terorist Knife Model"), &KNIFET);
				if (KNIFET)
					ImGui::InputText(XorStr("##Name4"), Options::SkinChanger::buf5, 128);

				ImGui::Text("Counter-Terorist Knife Model");
				ImGui::Checkbox(XorStr("Counter-Terorist Knife Model"), &KNIFECT);
				if (KNIFECT)
					ImGui::InputText(XorStr("##Name5"), Options::SkinChanger::buf6, 128);
			}
		}

		//int cw = /*U::SafeWeaponID();*/ 7;
		int cw = U::SafeWeaponID();
		if (cw == 0)
			return;
		if (U::IsWeaponDefaultKnife(cw))
			return;

		//ImGui::SetCursorPos(ImVec2(160, lPos.y));
		ImGui::Checkbox(XorStr("Enabled"), &weapons[cw].ChangerEnabled);

		ImGui::InputInt(XorStr("Skin Seed"), &weapons[cw].ChangerSeed);
		ImGui::SliderFloat(XorStr("Skin Wear"), &weapons[cw].ChangerWear, 0, 1);
		ImGui::InputText(XorStr("Skin Name"), weapons[cw].ChangerName, 32);
		ImGui::InputInt(XorStr("Skin StatTrak"), &weapons[cw].ChangerStatTrak);

		std::string skinName = U::GetWeaponNameById(cw);
		if (skinName.compare("") == 0)
			ImGui::InputInt(XorStr("Skin"), &weapons[cw].ChangerSkin);
		else
		{
			std::string skinStr = "";
			int curItem = -1;
			int s = 0;
			for (auto skin : G::weaponSkins[skinName])
			{
				int pk = G::skinMap[skin].paintkit;
				if (pk == weapons[cw].ChangerSkin)
					curItem = s;

				skinStr += G::skinNames[G::skinMap[skin].tagName].c_str();
				skinStr.push_back('\0');
				s++;
			}
			skinStr.push_back('\0');
			if (ImGui::Combo(XorStr("Skin"), &curItem, skinStr.c_str()))
			{
				int pk = 0;
				int c = 0;
				for (auto skin : G::weaponSkins[skinName])
				{
					if (curItem == c)
					{
						pk = G::skinMap[skin].paintkit;
						break;
					}

					c++;
				}
				weapons[cw].ChangerSkin = pk;
				if (weapons[cw].ChangerEnabled) U::FullUpdate();
			}
		}

			

		if (ImGui::Button(XorStr("Apply")))
			U::FullUpdate();
	}

	void DrawMiscTab()
	{
		ImGui::Text("Chams");
		ImGui::Checkbox(XorStr("Enemy only"), &Options::Visuals::Chams::EnemyOnly); ImGui::SameLine(); 			ImGui::Checkbox(XorStr("Visible only"), &Options::Visuals::Chams::VisibleOnly);
		
		ImGui::Checkbox(XorStr("Reveal Ranks"), &Options::Misc::RevealRanks);
		ImGui::Checkbox(XorStr("Recoil Crosshair"), &Options::Misc::RecoilCrosshair);
		ImGui::Checkbox(XorStr("Snipers Crosshair"), &Options::Misc::SniperCrosshair);
		ImGui::Checkbox(XorStr("Bunnyhop"), &Options::Misc::Bunnyhop);
		ImGui::Checkbox(XorStr("Custom UI"), &Options::Misc::SpectatorList);
		//ImGui::Checkbox(XorStr("Hit info"), &Options::Misc::HitInfo);
		ImGui::Checkbox(XorStr("Knife Bot"), &Options::Misc::KnifeBot);
		if (Options::Misc::KnifeBot)
		{
			ImGui::SameLine(); ImGui::Checkbox(XorStr("Auto"), &Options::Misc::KnifeBotAuto);
			ImGui::SameLine(); ImGui::Checkbox(XorStr("Silent"), &Options::Misc::KnifeBot360);
		}
		ImGui::Checkbox(XorStr("Anti AFK"), &Options::Misc::AntiAFK);
		ImGui::Checkbox(XorStr("Air Stuck (Left Alt)"), &Options::Misc::AirStuck);
		ImGui::Checkbox(XorStr("Watermark"), &Options::Misc::Watermark);	
		ImGui::SliderFloat(XorStr("Chams Rainbow Speed"), &Options::Misc::SControl, 0, 4, "%.3f", 1.0f, ImVec2(300, 0));
		ImGui::SliderFloat(XorStr("Hands Rainbow Speed"), &Options::Misc::HControl, 0, 4, "%.3f", 1.0f, ImVec2(300, 0));


		//ImGui::SetCursorPosY(curHeight - 100);
		//ImGui::Text(XorStr("Config:"));
		//if (ImGui::Button(XorStr("Load"), ImVec2(120, 0))) Config::Read();
		//if (ImGui::Button(XorStr("Save"), ImVec2(120, 0))) Config::Save();
	}

	void Render()
	{
		deltaTime = GetDeltaTime();

		ImGui::GetIO().MouseDrawCursor = G::MenuOpened;

		if (Options::Radar::Enabled && Options::Radar::Style == 0) DrawRadar();
		if (Options::Misc::SpectatorList) DrawSpectatorList();
		if (Options::Misc::Watermark) DrawWatermark();
		if (G::MenuOpened)
		{
			float deltaSize = 2 * deltaTime;
			if (curWidth < windowWidth)
				curWidth += min(windowWidth - curWidth, deltaSize);
			else if (curHeight < windowHeight)
				curHeight += min(windowHeight - curHeight, deltaSize);

			curX = (screenWidth / 2) - (curWidth / 2);
			curY = (screenHeight / 2) - (windowHeight / 2);

			int pX, pY;
			Interface.Input->GetCursorPosition(&pX, &pY);
			ImGuiIO& io = ImGui::GetIO();
			io.MousePos.x = (float)(pX);
			io.MousePos.y = (float)(pY);

			DrawTabs();
			if (curWidth >= windowWidth &&
				curHeight >= windowHeight)
			{
				if (legitTab) DrawLegitTab();
				if (visualsTab) DrawVisualsTab();
				if (changerTab) DrawChangerTab();
				if (radarTab) DrawRadarTab();
				if (colorsTab) DrawColorsTab();
				if (miscTab) DrawMiscTab();
			}

			ImGui::End();
		}
	}

	void OpenMenu()
	{
		static bool is_down = false;
		static bool is_clicked = false;
		static bool lastCursor = false;

		if (G::PressedKeys[VK_INSERT])
		{
			is_clicked = false;
			is_down = true;
		}

		else if (!G::PressedKeys[VK_INSERT] && is_down)
		{
			is_clicked = true;
			is_down = false;
		}
		else
		{
			is_clicked = false;
			is_down = false;
		}

		if (is_clicked)
		{
			curWidth = 40;
			curHeight = 40;
			curX = 0;
			curY = 0;

			G::MenuOpened = !G::MenuOpened;
			std::string msg = XorStr("cl_mouseenable ") + std::to_string(!G::MenuOpened);
			Interface.Engine->ExecuteConsoleCommand(msg.c_str());
		}
	}

	LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_LBUTTONDOWN:
			G::PressedKeys[VK_LBUTTON] = true;
			break;
		case WM_LBUTTONUP:
			G::PressedKeys[VK_LBUTTON] = false;
			break;
		case WM_RBUTTONDOWN:
			G::PressedKeys[VK_RBUTTON] = true;
			break;
		case WM_RBUTTONUP:
			G::PressedKeys[VK_RBUTTON] = false;
			break;
		case WM_KEYDOWN:
			G::PressedKeys[wParam] = true;
			break;
		case WM_KEYUP:
			G::PressedKeys[wParam] = false;
			break;
		default: break;
		}

		OpenMenu();

		if (G::Inited && G::MenuOpened && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		return CallWindowProc(g_pOldWindowProc, hWnd, uMsg, wParam, lParam);
	}
};