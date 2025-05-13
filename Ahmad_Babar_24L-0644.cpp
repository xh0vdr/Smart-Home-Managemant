#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace sf;

const int MAX_DEVICES = 50;
const int MAX_SHORTCUTS = 20;

class Device {
protected:
    string name, room;
    bool isOn;
public:
    Device() : name(""), room(""), isOn(false) {}
    Device(string n, string r) : name(n), room(r), isOn(false) {}
    virtual void toggle() {
        isOn = !isOn;
    }
    virtual string getStatus() const = 0;
    virtual string getType() const = 0;
    virtual ~Device() {}
    string getName() const {
        return name;
    }
    string getRoom() const {
        return room;
    }
    void setName(string n) {
        name = n;
    }
    void setRoom(string r) {
        room = r;
    }
    bool getState() const {
        return isOn;
    }
    void setState(bool s) {
        isOn = s;
    }
};

class Light : public Device {
public:
    Light() : Device() {}
    Light(string n, string r) : Device(n, r) {}
    string getStatus() const override { 
        return isOn ? "Light is ON" : "Light is OFF"; 
    }
    string getType() const override { 
        return "Light"; 
    }
};

class Fan : public Device {
public:
    Fan() : Device() {}
    Fan(string n, string r) : Device(n, r) {}
    string getStatus() const override { 
        return isOn ? "Fan is ON" : "Fan is OFF"; 
    }
    string getType() const override { 
        return "Fan";
    }
};

class AC : public Device {
public:
    AC() : Device() {}
    AC(string n, string r) : Device(n, r) {}
    string getStatus() const override { 
        return isOn ? "AC is ON" : "AC is OFF"; 
    }
    string getType() const override {
        return "AC"; 
    }
};

class TV : public Device {
public:
    TV() : Device() {}
    TV(string n, string r) : Device(n, r) {}
    string getStatus() const override { 
        return isOn ? "TV is ON" : "TV is OFF"; 
    }
    string getType() const override { 
        return "TV"; 
    }
};

class Fridge : public Device {
public:
    Fridge() : Device() {}
    Fridge(string n, string r) : Device(n, r) {}
    string getStatus() const override {
        return isOn ? "Fridge is COOLING" : "Fridge is OFF"; 
    }
    string getType() const override { 
        return "Fridge"; 
    }
};

class Door : public Device {
public:
    Door() : Device() {}
    Door(string n, string r) : Device(n, r) {}
    string getStatus() const override { 
        return isOn ? "Door is UNLOCKED" : "Door is LOCKED";
    }
    string getType() const override { 
        return "Door";
    }
};

class Shortcut {
public:
    string name;
    int deviceIndices[MAX_DEVICES];
    int indexCount;
    Shortcut(string n) : name(n), indexCount(0) {
        for (int i = 0; i < MAX_DEVICES; ++i) deviceIndices[i] = -1;
    }
    void addDevice(int index) {
        if (indexCount < MAX_DEVICES) {
            deviceIndices[indexCount++] = index;
        }
    }
    void toggleDevices(Device* devices[], int deviceCount) {
        for (int i = 0; i < indexCount; ++i) {
            int idx = deviceIndices[i];
            if (idx >= 0 && idx < deviceCount && devices[idx]) {
                devices[idx]->toggle();
            }
        }
    }
};

Device* createDevice(string type, string name, string room) {
    if (type == "Light") 
        return new Light(name, room);
    if (type == "Fan") 
        return new Fan(name, room);
    if (type == "AC") 
        return new AC(name, room);
    if (type == "TV") 
        return new TV(name, room);
    if (type == "Fridge") 
        return new Fridge(name, room);
    if (type == "Door") 
        return new Door(name, room);

    return nullptr;
}

int main() {
    bool isDarkMode = false;

    string tabs[4] = { "Home", "Scheduled", "Settings", "Statistics" };
    int currentTab = 0;

    RenderWindow window(VideoMode(1000, 800), "Smart Home Manager");
    Font font;
    if (!font.loadFromFile("ariblk.ttf")) {
             return 1;
    }

    string storedUser = "admin", storedPass = "1234";
    string inputUser = "", inputPass = "";
    bool loginScreen = true, typingPassword = false;

    Device* devices[MAX_DEVICES] = { nullptr };
    int deviceCount = 0;

    Shortcut* shortcuts[MAX_SHORTCUTS] = { nullptr };
    int shortcutCount = 0;

    string inputName = "", inputRoom = "", inputShortcutName = "";
    string deviceTypes[6] = { "Light", "Fan", "AC", "TV", "Fridge", "Door" };
    int selectedType = 0;
    bool inEditMode = false, inAddMode = false, typingRoom = false;
    bool inAddShortcutMode = false;
    int editIndex = -1;
    float scrollOffset = 0;
    bool selectedDevices[MAX_DEVICES] = { false };

    while (window.isOpen()) {
        Event event;
        bool mouseClicked = false;
        Vector2f mousePos;

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) window.close();
            if (event.type == Event::MouseWheelScrolled) {
                scrollOffset -= event.mouseWheelScroll.delta * 30;
                if (scrollOffset < 0) scrollOffset = 0;
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                mouseClicked = true;
                mousePos = Vector2f(event.mouseButton.x, event.mouseButton.y);
            }
            if ((inAddMode || inEditMode) && event.type == Event::TextEntered) {
                if (event.text.unicode == 8) {
                    if (typingRoom && !inputRoom.empty()) inputRoom.pop_back();
                    else if (!typingRoom && !inputName.empty()) inputName.pop_back();
                }
                else if (event.text.unicode < 128 && event.text.unicode != 13) {
                    if (!typingRoom && inputName.length() < 20) inputName += static_cast<char>(event.text.unicode);
                    else if (typingRoom && inputRoom.length() < 20) inputRoom += static_cast<char>(event.text.unicode);
                }
            }
            if (inAddShortcutMode && event.type == Event::TextEntered) {
                if (event.text.unicode == 8 && !inputShortcutName.empty()) {
                    inputShortcutName.pop_back();
                }
                else if (event.text.unicode < 128 && event.text.unicode != 13 && inputShortcutName.length() < 20) {
                    inputShortcutName += static_cast<char>(event.text.unicode);
                }
            }
            if (loginScreen && event.type == Event::TextEntered) {
                if (event.text.unicode == 8) {
                    if (typingPassword && !inputPass.empty()) inputPass.pop_back();
                    else if (!typingPassword && !inputUser.empty()) inputUser.pop_back();
                }
                else if (event.text.unicode < 128 && event.text.unicode != 13) {
                    if (typingPassword && inputPass.size() < 20) inputPass += static_cast<char>(event.text.unicode);
                    else if (!typingPassword && inputUser.size() < 20) inputUser += static_cast<char>(event.text.unicode);
                }
            }
        }

        Color lightBG(220, 220, 220), darkBG(40, 40, 40);
        Color lightCard(240, 240, 240), darkCard(60, 60, 60);
        Color lightText = Color::Black, darkText = Color::White;
        Color shadowColor(0, 0, 0, 50);

        Color bgColor = isDarkMode ? darkBG : lightBG;
        Color cardColor = isDarkMode ? darkCard : lightCard;
        Color textColor = isDarkMode ? darkText : lightText;

        window.clear(bgColor);

        if (loginScreen) {
            RectangleShape loginShadow(Vector2f(600, 300));
            loginShadow.setPosition(205, 205);
            loginShadow.setFillColor(shadowColor);
            window.draw(loginShadow);

            RectangleShape loginBox(Vector2f(600, 300));
            loginBox.setFillColor(Color(230, 230, 230));
            loginBox.setOutlineColor(Color::White);
            loginBox.setOutlineThickness(2);
            loginBox.setPosition(200, 200);
            window.draw(loginBox);

            Text title("Login to Smart Home", font, 30);
            title.setFillColor(textColor);
            title.setPosition(300, 220);
            window.draw(title);

            Text uLabel("Username:", font, 22);
            uLabel.setFillColor(Color::Black);
            uLabel.setPosition(220, 280);
            window.draw(uLabel);

            RectangleShape uBox(Vector2f(400, 35));
            uBox.setFillColor(Color::White);
            uBox.setPosition(330, 280);
            window.draw(uBox);
            Text uText(inputUser, font, 22);
            uText.setFillColor(Color::Black);
            uText.setPosition(335, 282);
            window.draw(uText);

            Text pLabel("Password:", font, 22);
            pLabel.setFillColor(Color::Black);
            pLabel.setPosition(220, 340);
            window.draw(pLabel);

            RectangleShape pBox(Vector2f(400, 35));
            pBox.setFillColor(Color::White);
            pBox.setPosition(330, 340);
            window.draw(pBox);
            Text pText(string(inputPass.length(), '*'), font, 22);
            pText.setFillColor(Color::Black);
            pText.setPosition(335, 342);
            window.draw(pText);

            RectangleShape loginBtn(Vector2f(160, 45));
            loginBtn.setFillColor(Color(255, 140, 0));
            loginBtn.setOutlineColor(Color::White);
            loginBtn.setOutlineThickness(2);
            loginBtn.setPosition(420, 400);
            window.draw(loginBtn);
            Text loginText("Login", font, 22);
            loginText.setFillColor(Color::White);
            loginText.setPosition(465, 408);
            window.draw(loginText);

            if (mouseClicked) {
                if (FloatRect(330, 280, 400, 35).contains(mousePos))
                    typingPassword = false;
                else if (FloatRect(330, 340, 400, 35).contains(mousePos)) 
                    typingPassword = true;
                else if (FloatRect(420, 400, 160, 45).contains(mousePos)) {
                    if (inputUser == storedUser && inputPass == storedPass) {
                        loginScreen = false;
                        devices[deviceCount++] = new Light("Wall Light", "Bedroom");
                    }
                }
            }
            window.display();
            continue;
        }

        VertexArray topBar(Quads, 4);
        topBar[0].position = Vector2f(0, 0);
        topBar[1].position = Vector2f(1000, 0);
        topBar[2].position = Vector2f(1000, 60);
        topBar[3].position = Vector2f(0, 60);
        topBar[0].color = Color(30, 30, 30);
        topBar[1].color = Color(30, 30, 30);
        topBar[2].color = Color(255, 140, 0);
        topBar[3].color = Color(255, 140, 0);
        window.draw(topBar);

        for (int i = 0; i < 4; ++i) {
            RectangleShape tab(Vector2f(160, 45));
            tab.setPosition(20 + i * 180, 7);
            tab.setFillColor(i == currentTab ? Color(255, 140, 0) : Color(80, 80, 80));
            tab.setOutlineColor(Color::White);
            tab.setOutlineThickness(2);
            if (FloatRect(20 + i * 180, 7, 160, 45).contains(mousePos)) {
                tab.setFillColor(i == currentTab ? Color(255, 160, 20) : Color(100, 100, 100));
            }
            window.draw(tab);

            Text tabLabel(tabs[i], font, 20);
            tabLabel.setFillColor(Color::White);
            tabLabel.setPosition(50 + i * 180, 15);
            window.draw(tabLabel);

            if (mouseClicked && tab.getGlobalBounds().contains(mousePos)) {
                currentTab = i;
            }
        }

        if (currentTab == 0) { // Home tab
            RectangleShape addBtn(Vector2f(160, 45));
            addBtn.setFillColor(Color(255, 140, 0));
            addBtn.setOutlineColor(Color::White);
            addBtn.setOutlineThickness(2);
            addBtn.setPosition(50, 80);
            window.draw(addBtn);
            Text addLabel("Add Device", font, 20);
            addLabel.setFillColor(Color::White);
            addLabel.setPosition(65, 88);
            window.draw(addLabel);

            if (mouseClicked && FloatRect(50, 80, 160, 45).contains(mousePos)) {
                inAddMode = true;
                inputName = inputRoom = "";
            }

            float y = 150 - scrollOffset;
            for (int i = 0; i < deviceCount; ++i) {
                if (!devices[i]) 
                    continue;

                RectangleShape shadow(Vector2f(820, 110));
                shadow.setPosition(105, y + 5);
                shadow.setFillColor(shadowColor);
                window.draw(shadow);

                RectangleShape card(Vector2f(820, 110));
                card.setPosition(100, y);
                card.setFillColor(cardColor);
                card.setOutlineColor(Color::White);
                card.setOutlineThickness(1);
                window.draw(card);

                Text name("Device: " + devices[i]->getName(), font, 20);
                name.setFillColor(Color::Black);
                name.setPosition(120, y + 15);
                window.draw(name);

                Text status(devices[i]->getStatus(), font, 18);
                status.setFillColor(Color::Black);
                status.setPosition(120, y + 45);
                window.draw(status);

                Text room("Room: " + devices[i]->getRoom(), font, 18);
                room.setFillColor(Color::Black);
                room.setPosition(120, y + 75);
                window.draw(room);

                RectangleShape edit(Vector2f(70, 35));
                edit.setFillColor(Color(70, 130, 180));
                edit.setPosition(850, y + 15);
                window.draw(edit);
                Text etxt("Edit", font, 16);
                etxt.setFillColor(Color::White);
                etxt.setPosition(860, y + 20);
                window.draw(etxt);

                RectangleShape del(Vector2f(70, 35));
                del.setFillColor(Color(200, 40, 40));
                del.setPosition(850, y + 60);
                window.draw(del);
                Text dtxt("Delete", font, 16);
                dtxt.setFillColor(Color::White);
                dtxt.setPosition(855, y + 65);
                window.draw(dtxt);

                if (mouseClicked && FloatRect(100, y, 820, 110).contains(mousePos)) 
                    devices[i]->toggle();
                if (mouseClicked && FloatRect(850, y + 15, 70, 35).contains(mousePos)) {
                    inEditMode = true;
                    editIndex = i;
                    inputName = devices[i]->getName();
                    inputRoom = devices[i]->getRoom();
                    string type = devices[i]->getType();
                    for (int t = 0; t < 6; ++t) if (deviceTypes[t] == type)
                        selectedType = t;
                }
                if (mouseClicked && FloatRect(850, y + 60, 70, 35).contains(mousePos)) {
                    delete devices[i];
                    devices[i] = nullptr;
                    deviceCount--;
                    for (int j = i; j < deviceCount; ++j) {
                        devices[j] = devices[j + 1];
                    }
                    devices[deviceCount] = nullptr;
                    for (int s = 0; s < shortcutCount; ++s) {
                        if (shortcuts[s]) {
                            for (int j = 0; j < shortcuts[s]->indexCount; ++j) {
                                if (shortcuts[s]->deviceIndices[j] == i) {
                                    shortcuts[s]->deviceIndices[j] = -1;
                                }
                                if (shortcuts[s]->deviceIndices[j] > i) {
                                    shortcuts[s]->deviceIndices[j]--;
                                }
                            }
                            int newCount = 0;
                            for (int j = 0; j < shortcuts[s]->indexCount; ++j) {
                                if (shortcuts[s]->deviceIndices[j] != -1) {
                                    shortcuts[s]->deviceIndices[newCount++] = shortcuts[s]->deviceIndices[j];
                                }
                            }
                            shortcuts[s]->indexCount = newCount;
                            for (int j = shortcuts[s]->indexCount; j < MAX_DEVICES; ++j) {
                                shortcuts[s]->deviceIndices[j] = -1;
                            }
                        }
                    }
                }
                y += 130;
            }
        }
        else if (currentTab == 1) { // Scheduled tab
            Text scheduleTitle("Scheduled Shortcuts", font, 30);
            scheduleTitle.setFillColor(textColor);
            scheduleTitle.setPosition(50, 80);
            window.draw(scheduleTitle);

            RectangleShape addShortcutBtn(Vector2f(160, 45));
            addShortcutBtn.setFillColor(Color(255, 140, 0));
            addShortcutBtn.setOutlineColor(Color::White);
            addShortcutBtn.setOutlineThickness(2);
            addShortcutBtn.setPosition(50, 140);
            window.draw(addShortcutBtn);
            Text addShortcutLabel("Add Shortcut", font, 20);
            addShortcutLabel.setFillColor(Color::White);
            addShortcutLabel.setPosition(65, 148);
            window.draw(addShortcutLabel);

            if (mouseClicked && FloatRect(50, 140, 160, 45).contains(mousePos)) {
                inAddShortcutMode = true;
                inputShortcutName = "";
                for (int i = 0; i < MAX_DEVICES; ++i) selectedDevices[i] = false;
            }

            float y = 210 - scrollOffset;
            if (shortcutCount == 0) {
                Text noShortcuts("No shortcuts created yet.", font, 22);
                noShortcuts.setFillColor(textColor);
                noShortcuts.setPosition(50, 210);
                window.draw(noShortcuts);
            }
            else {
                for (int i = 0; i < shortcutCount; ++i) {
                    if (!shortcuts[i]) 
                        continue;
                    RectangleShape shadow(Vector2f(820, 70));
                    shadow.setPosition(105, y + 5);
                    shadow.setFillColor(shadowColor);
                    window.draw(shadow);

                    RectangleShape card(Vector2f(820, 70));
                    card.setPosition(100, y);
                    card.setFillColor(cardColor);
                    card.setOutlineColor(Color::White);
                    card.setOutlineThickness(1);
                    window.draw(card);

                    Text name("Shortcut: " + shortcuts[i]->name, font, 20);
                    name.setFillColor(Color::Black);
                    name.setPosition(120, y + 15);
                    window.draw(name);

                    string deviceList = "Devices: ";
                    for (int j = 0; j < shortcuts[i]->indexCount; ++j) {
                        int idx = shortcuts[i]->deviceIndices[j];
                        if (idx >= 0 && idx < deviceCount && devices[idx]) {
                            deviceList += devices[idx]->getName() + ", ";
                        }
                    }
                    if (deviceList.length() > 10) 
                        deviceList = deviceList.substr(0, deviceList.length() - 2);
                    else
                        deviceList = "Devices: None";

                    Text devicesText(deviceList, font, 18);
                    devicesText.setFillColor(Color::Black);
                    devicesText.setPosition(120, y + 40);
                    window.draw(devicesText);

                    if (mouseClicked && FloatRect(100, y, 820, 70).contains(mousePos)) {
                        shortcuts[i]->toggleDevices(devices, deviceCount);
                    }

                    y += 90;
                }
            }
        }
        else if (currentTab == 2) { // Settings tab
            Text settingsTitle("Settings", font, 30);
            settingsTitle.setFillColor(textColor);
            settingsTitle.setPosition(50, 80);
            window.draw(settingsTitle);

            Text credits("CREDITS:\n\tMade by the GOAT Ahmad himself", font, 22);
            credits.setFillColor(textColor);
            credits.setPosition(50, 140);
            window.draw(credits);

            RectangleShape toggleBtn(Vector2f(260, 45));
            toggleBtn.setFillColor(isDarkMode ? Color(100, 100, 255) : Color(30, 30, 150));
            toggleBtn.setOutlineColor(Color::White);
            toggleBtn.setOutlineThickness(2);
            toggleBtn.setPosition(50, 210);
            window.draw(toggleBtn);

            Text toggleText(isDarkMode ? "Switch to Light Mode" : "Switch to Dark Mode", font, 20);
            toggleText.setFillColor(Color::White);
            toggleText.setPosition(60, 218);
            window.draw(toggleText);

            if (mouseClicked && toggleBtn.getGlobalBounds().contains(mousePos)) {
                isDarkMode = !isDarkMode;
            }
        }
        else if (currentTab == 3) { // Statistics tab
            Text title("System Statistics", font, 30);
            title.setFillColor(textColor);
            title.setPosition(50, 80);
            window.draw(title);

            int totalOn = 0, totalOff = 0;
            int lightCount = 0, fanCount = 0, acCount = 0, tvCount = 0, fridgeCount = 0, doorCount = 0;
            string devicesOn = "Devices ON: ";
            string devicesOff = "Devices OFF: ";

            for (int i = 0; i < deviceCount; ++i) {
                if (!devices[i]) continue;
                if (devices[i]->getState()) {
                    totalOn++;
                    devicesOn += devices[i]->getName() + ", ";
                }
                else {
                    totalOff++;
                    devicesOff += devices[i]->getName() + ", ";
                }

                string type = devices[i]->getType();
                if (type == "Light") 
                    lightCount++;
                else if (type == "Fan") 
                    fanCount++;
                else if (type == "AC") 
                    acCount++;
                else if (type == "TV")
                    tvCount++;
                else if (type == "Fridge") 
                    fridgeCount++;
                else if (type == "Door") 
                    doorCount++;
            }

            if (!devicesOn.empty() && devicesOn[devicesOn.length() - 2] == ',') {
                devicesOn = devicesOn.substr(0, devicesOn.length() - 2);
            }
            if (!devicesOff.empty() && devicesOff[devicesOff.length() - 2] == ',') {
                devicesOff = devicesOff.substr(0, devicesOff.length() - 2);
            }

            string stats[] = {
                "Total Devices: " + to_string(deviceCount),
                "Devices ON: " + to_string(totalOn),
                "Devices OFF: " + to_string(totalOff),
                devicesOn.empty() ? "Devices ON: None" : devicesOn,
                devicesOff.empty() ? "Devices OFF: None" : devicesOff,
                "Lights: " + to_string(lightCount),
                "Fans: " + to_string(fanCount),
                "ACs: " + to_string(acCount),
                "TVs: " + to_string(tvCount),
                "Fridges: " + to_string(fridgeCount),
                "Doors: " + to_string(doorCount)
            };

            float y = 150;
            for (const string& s : stats) {
                Text line(s, font, 22);
                line.setFillColor(textColor);
                line.setPosition(50, y);
                window.draw(line);
                y += 40;
            }
        }

        if (inAddMode || inEditMode) {
            RectangleShape popup(Vector2f(500, 360));
            popup.setFillColor(Color(230, 230, 230));
            popup.setOutlineColor(Color::White);
            popup.setOutlineThickness(2);
            popup.setPosition(250, 220);
            window.draw(popup);

            Text title(inAddMode ? "Add Device" : "Edit Device", font, 26);
            title.setFillColor(textColor);
            title.setPosition(360, 230);
            window.draw(title);

            Text nameLabel("Device Name:", font, 20);
            nameLabel.setFillColor(Color::Black);
            nameLabel.setPosition(270, 270);
            window.draw(nameLabel);

            RectangleShape nameBox(Vector2f(400, 35));
            nameBox.setFillColor(Color::White);
            nameBox.setPosition(270, 300);
            window.draw(nameBox);
            Text nameText(inputName, font, 20);
            nameText.setFillColor(Color::Black);
            nameText.setPosition(275, 302);
            window.draw(nameText);

            Text roomLabel("Room Name:", font, 20);
            roomLabel.setFillColor(Color::Black);
            roomLabel.setPosition(270, 340);
            window.draw(roomLabel);

            RectangleShape roomBox(Vector2f(400, 35));
            roomBox.setFillColor(Color::White);
            roomBox.setPosition(270, 370);
            window.draw(roomBox);
            Text roomText(inputRoom, font, 20);
            roomText.setFillColor(Color::Black);
            roomText.setPosition(275, 372);
            window.draw(roomText);

            Text typeLabel("Device Type:", font, 20);
            typeLabel.setFillColor(Color::Black);
            typeLabel.setPosition(270, 410);
            window.draw(typeLabel);

            RectangleShape typeBox(Vector2f(400, 35));
            typeBox.setFillColor(Color::White);
            typeBox.setPosition(270, 440);
            window.draw(typeBox);
            Text typeText(deviceTypes[selectedType], font, 20);
            typeText.setFillColor(Color::Black);
            typeText.setPosition(275, 442);
            window.draw(typeText);

            RectangleShape done(Vector2f(160, 45));
            done.setFillColor(Color(0, 120, 60));
            done.setPosition(400, 490);
            window.draw(done);
            Text doneLabel(inAddMode ? "Add" : "Save", font, 22);
            doneLabel.setFillColor(Color::White);
            doneLabel.setPosition(455, 498);
            window.draw(doneLabel);

            if (mouseClicked) {
                if (FloatRect(270, 300, 400, 35).contains(mousePos))
                    typingRoom = false;
                else if (FloatRect(270, 370, 400, 35).contains(mousePos)) 
                    typingRoom = true;
                else if (FloatRect(270, 440, 400, 35).contains(mousePos))
                    selectedType = (selectedType + 1) % 6;
                else if (FloatRect(400, 490, 160, 45).contains(mousePos)) {
                    if (!inputName.empty() && !inputRoom.empty()) {
                        Device* newDevice = createDevice(deviceTypes[selectedType], inputName, inputRoom);
                        if (newDevice) {
                            if (inEditMode) {
                                delete devices[editIndex];
                                devices[editIndex] = newDevice;
                            }
                            else if (inAddMode && deviceCount < MAX_DEVICES) {
                                devices[deviceCount++] = newDevice;
                            }
                            inAddMode = inEditMode = false;
                            inputName = inputRoom = "";
                        }
                    }
                }
            }
        }

        if (inAddShortcutMode) {
            RectangleShape popup(Vector2f(500, 400 + deviceCount * 30));
            popup.setFillColor(Color(230, 230, 230));
            popup.setOutlineColor(Color::White);
            popup.setOutlineThickness(2);
            popup.setPosition(250, 200);
            window.draw(popup);

            Text title("Create Shortcut", font, 26);
            title.setFillColor(textColor);
            title.setPosition(360, 210);
            window.draw(title);

            Text nameLabel("Shortcut Name:", font, 20);
            nameLabel.setFillColor(Color::Black);
            nameLabel.setPosition(270, 250);
            window.draw(nameLabel);

            RectangleShape nameBox(Vector2f(400, 35));
            nameBox.setFillColor(Color::White);
            nameBox.setPosition(270, 280);
            window.draw(nameBox);
            Text nameText(inputShortcutName, font, 20);
            nameText.setFillColor(Color::Black);
            nameText.setPosition(275, 282);
            window.draw(nameText);

            Text devicesLabel("Select Devices:", font, 20);
            devicesLabel.setFillColor(Color::Black);
            devicesLabel.setPosition(270, 320);
            window.draw(devicesLabel);

            float y = 350;
            for (int i = 0; i < deviceCount; ++i) {
                if (!devices[i]) continue;
                RectangleShape checkbox(Vector2f(22, 22));
                checkbox.setPosition(270, y);
                checkbox.setFillColor(selectedDevices[i] ? Color(0, 120, 60) : Color::White);
                checkbox.setOutlineColor(Color::Black);
                checkbox.setOutlineThickness(1);
                window.draw(checkbox);

                Text deviceName(devices[i]->getName() + " (" + devices[i]->getType() + ")", font, 18);
                deviceName.setFillColor(Color::Black);
                deviceName.setPosition(300, y);
                window.draw(deviceName);

                if (mouseClicked && FloatRect(270, y, 22, 22).contains(mousePos)) {
                    selectedDevices[i] = !selectedDevices[i];
                }
                y += 30;
            }

            RectangleShape done(Vector2f(160, 45));
            done.setFillColor(Color(0, 120, 60));
            done.setPosition(400, y + 20);
            window.draw(done);
            Text doneLabel("Create", font, 22);
            doneLabel.setFillColor(Color::White);
            doneLabel.setPosition(435, y + 28);
            window.draw(doneLabel);

            if (mouseClicked && FloatRect(400, y + 20, 160, 45).contains(mousePos)) {
                if (!inputShortcutName.empty() && shortcutCount < MAX_SHORTCUTS) {
                    shortcuts[shortcutCount] = new Shortcut(inputShortcutName);
                    for (int i = 0; i < deviceCount; ++i) {
                        if (selectedDevices[i]) shortcuts[shortcutCount]->addDevice(i);
                    }
                    if (shortcuts[shortcutCount]->indexCount > 0) {
                        shortcutCount++;
                    }
                    else {
                        delete shortcuts[shortcutCount];
                        shortcuts[shortcutCount] = nullptr;
                    }
                    inAddShortcutMode = false;
                    inputShortcutName = "";
                    for (int i = 0; i < MAX_DEVICES; ++i) selectedDevices[i] = false;
                }
            }
        }

        window.display();
    }

    for (int i = 0; i < deviceCount; ++i) {
        if (devices[i]) delete devices[i];
    }
    for (int i = 0; i < shortcutCount; ++i) {
        if (shortcuts[i]) delete shortcuts[i];
    }
    return 0;
}