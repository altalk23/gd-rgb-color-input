#define CAC_PROJ_NAME "RGBColorInput"
#include <CacKit>

std::string colorToHex(ccColor3B color) {
    static constexpr auto digits = "0123456789ABCDEF";
    std::string output;
    output += digits[color.r >> 4 & 0xF];
    output += digits[color.r      & 0xF];
    output += digits[color.g >> 4 & 0xF];
    output += digits[color.g      & 0xF];
    output += digits[color.b >> 4 & 0xF];
    output += digits[color.b      & 0xF];
    return output;
}

template <typename T>
std::string to_string(T val) {
    std::stringstream stream;
    stream << val;
    return stream.str();
}

class RGBColorInputWidget : public cocos2d::CCLayer, public TextInputDelegate {
    ColorSelectPopup* parent;
    CCTextInputNode* redInput;
    CCTextInputNode* greenInput;
    CCTextInputNode* blueInput;
    CCTextInputNode* hexInput;

    auto getPickerColor() { 
        return *reinterpret_cast<ccColor3B*>(reinterpret_cast<uintptr_t>(parent->_colorPicker()) + 0x1c8); 
    }

    void setPickerColor(ccColor3B color) { 
        // headers broken smh
        reinterpret_cast<void(*)(extension::CCControlColourPicker*, ccColor3B*)>(*(uint64_t*)(*(uint64_t*)parent->_colorPicker() + 0x518))(parent->_colorPicker(), &color);
    }

    bool init(ColorSelectPopup* parent) {
        if (!CCLayer::init()) return false;
        this->parent = parent;

        const ccColor3B placeholderColor = {127, 127, 127};

        constexpr float totalWidth = 115.f;
        constexpr float spacing = 4.f;
        constexpr float compWidth = (totalWidth - spacing * 2.f) / 3.f; // components (R G B) width
        constexpr float compHeight = 22.5f;
        constexpr float hexHeight = 30.f;
        constexpr float hexY = -hexHeight - spacing / 2.f;
        constexpr float rXPos = -compWidth - spacing;
        constexpr float bXPos = -rXPos;
        constexpr float bgScale = 1.6f;
        constexpr GLubyte opacity = 100;

        redInput = CCTextInputNode::create(30.0, 20.0, "R", "Thonburi", 24, "bigFont.fnt");
        redInput->setAllowedChars("0123456789");
        redInput->m_maxLabelLength = 3;
        redInput->setMaxLabelScale(0.6f);
        redInput->setLabelPlaceholderColor(placeholderColor);
        redInput->setLabelPlaceholderScale(0.5f);
        redInput->setPositionX(rXPos);
        redInput->m_delegate = this;

        greenInput = CCTextInputNode::create(30.0, 20.0, "G", "Thonburi", 24, "bigFont.fnt");
        greenInput->setAllowedChars("0123456789");
        greenInput->m_maxLabelLength = 3;
        greenInput->setMaxLabelScale(0.6f);
        greenInput->setLabelPlaceholderColor(placeholderColor);
        greenInput->setLabelPlaceholderScale(0.5f);
        greenInput->setPositionX(0.f);
        greenInput->m_delegate = this;
        
        blueInput = CCTextInputNode::create(30.0, 20.0, "B", "Thonburi", 24, "bigFont.fnt");
        blueInput->setAllowedChars("0123456789");
        blueInput->m_maxLabelLength = 3;
        blueInput->setMaxLabelScale(0.6f);
        blueInput->setLabelPlaceholderColor(placeholderColor);
        blueInput->setLabelPlaceholderScale(0.5f);
        blueInput->setPositionX(bXPos);
        blueInput->m_delegate = this;

        hexInput = CCTextInputNode::create(100.0, 20.0, "hex", "Thonburi", 24, "bigFont.fnt");
        hexInput->setAllowedChars("0123456789ABCDEFabcdef");
        hexInput->m_maxLabelLength = 6;
        hexInput->setMaxLabelScale(0.7f);
        hexInput->setLabelPlaceholderColor(placeholderColor);
        hexInput->setLabelPlaceholderScale(0.5f);
        hexInput->setPositionY(hexY);
        hexInput->m_delegate = this;

        addChild(redInput);
        addChild(greenInput);
        addChild(blueInput);
        addChild(hexInput);

        updateLabels(true, true);

        auto bg = extension::CCScale9Sprite::create("square02_small.png");
        bg->setContentSize({totalWidth * bgScale, hexHeight * bgScale});
        bg->setScale(1.f / bgScale);
        bg->setOpacity(opacity);
        bg->setZOrder(-1);
        bg->setPositionY(hexY);
        addChild(bg);

        bg = extension::CCScale9Sprite::create("square02_small.png");
        bg->setContentSize({compWidth * bgScale, compHeight * bgScale});
        bg->setScale(1.f / bgScale);
        bg->setOpacity(opacity);
        bg->setZOrder(-1);
        bg->setPositionX(rXPos);
        addChild(bg);

        bg = extension::CCScale9Sprite::create("square02_small.png");
        bg->setContentSize({compWidth * bgScale, compHeight * bgScale});
        bg->setScale(1.f / bgScale);
        bg->setOpacity(opacity);
        bg->setZOrder(-1);
        addChild(bg);

        bg = extension::CCScale9Sprite::create("square02_small.png");
        bg->setContentSize({compWidth * bgScale, compHeight * bgScale});
        bg->setScale(1.f / bgScale);
        bg->setOpacity(opacity);
        bg->setZOrder(-1);
        bg->setPositionX(bXPos);
        addChild(bg);

        return true;
    }

    bool ignore = false; // lmao this is such a hacky fix

    virtual void textChanged(CCTextInputNode* input) {
        if (ignore) return;
        if (input == hexInput) {
            std::string value(input->getString());
            ccColor3B color;
            if (value.empty()) return;
            auto num_value = std::strtol(value.c_str(), 0, 16);
            if (value.size() == 6) {
                // please shut up about narrowing conversions
                auto r = static_cast<uint8_t>((num_value & 0xFF0000) >> 16);
                auto g = static_cast<uint8_t>((num_value & 0x00FF00) >> 8);
                auto b = static_cast<uint8_t>((num_value & 0x0000FF));
                color = {r, g, b};
            } else if (value.size() == 2) {
                auto number = static_cast<uint8_t>(num_value); // please shut up c++
                color = {number, number, number};
            } else {
                return;
            }
            ignore = true;
            setPickerColor(color);
            ignore = false;
            updateLabels(false, true);
        } else if (input == redInput || input == greenInput || input == blueInput) {
            std::string value(input->getString());
            auto _num = value.empty() ? 0 : std::atoi(value.c_str());
            if (_num > 255) {
                _num = 255;
                input->setString("255");
            }
            GLubyte num = static_cast<GLubyte>(_num);
            auto color = getPickerColor();
            if (input == redInput)
                color.r = num;
            else if (input == greenInput)
                color.g = num;
            else if (input == blueInput)
                color.b = num;
            ignore = true;
            setPickerColor(color);
            ignore = false;
            updateLabels(true, false);
        }
    }

public:
    void updateLabels(bool hex, bool rgb) {
        if (ignore) return;
        ignore = true;
        auto color = getPickerColor();
        if (hex) {
            hexInput->setString(colorToHex(color));
        }
        if (rgb) {
            redInput->setString(to_string((int)(color.r)));
            greenInput->setString(to_string((int)(color.g)));
            blueInput->setString(to_string((int)(color.b)));
        }
        ignore = false;
    }

    static auto create(ColorSelectPopup* parent) {
        auto pRet = new RGBColorInputWidget();
        if (pRet && pRet->init(parent)) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = 0;
            return pRet;
        }
    }
};

RGBColorInputWidget* colorInputWidget = nullptr;

class $redirect(ColorSelectPopup) {
public:
    bool init(EffectGameObject *p0, CCArray *p1, ColorAction *p2) override {
        // std::cout << "ColorSelectPopup::init\n";
        colorInputWidget = nullptr;

        if (!$ColorSelectPopup::init(p0, p1, p2)) return false;

        auto layer = m_mainLayer;
        auto widget = RGBColorInputWidget::create(this);
        colorInputWidget = widget;
        auto center = WINSIZE / 2.f;
        if (_isColorTrigger())
            widget->setPosition({center.width - 155.f, center.height + 29.f});
        else
            widget->setPosition({center.width + 127.f, center.height - 90.f});
        layer->addChild(widget);
        widget->setVisible(!_copyColor());

        return true;
    }

    void updateCopyColor() {
        // std::cout << "ColorSelectPopup::updateCopyColor\n";
        $ColorSelectPopup::updateCopyColor();
        if (colorInputWidget) colorInputWidget->setVisible(!_copyColor());
    }

    void colorValueChanged(cocos2d::_ccColor3B p0) {
        // std::cout << "ColorSelectPopup::colorValueChanged" << std::endl;
        $ColorSelectPopup::colorValueChanged(p0);
        if (colorInputWidget) colorInputWidget->updateLabels(true, true);
    }

    // void dtor() override {
    //     // std::cout << "ColorSelectPopup::~ColorSelectPopup\n";
    //     colorInputWidget = nullptr;
    //     $ColorSelectPopup::dtor();
    // }
} ColorSelectHook;

class $redirect(SetupPulsePopup) {
public:
    bool init(EffectGameObject *p0, CCArray *p1) override {
        // std::cout << "SetupPulsePopup::init\n";
        // std::cout << this << std::endl;
        colorInputWidget = nullptr;

        if (!$SetupPulsePopup::init(p0, p1)) return false;

        auto layer = m_mainLayer;
        auto widget = RGBColorInputWidget::create(reinterpret_cast<ColorSelectPopup*>(this));

        colorInputWidget = widget;
        auto center = WINSIZE / 2.f;

        _colorPicker()->setPositionX(_colorPicker()->getPositionX() + 3.7f);
        widget->setPosition({center.width - 132.f, center.height + 32.f});
        auto squareWidth = _currentColorSpr()->getScaledContentSize().width;
        auto x = widget->getPositionX() - squareWidth / 2.f;
        _currentColorSpr()->setPosition({x, center.height + 85.f});
        _prevColorSpr()->setPosition({x + squareWidth, center.height + 85.f});

        layer->addChild(widget);
        widget->setVisible(_pulseMode() == 0);

        return true;
    }

    void onSelectPulseMode(CCObject *p0) {
        // std::cout << "SetupPulsePopup::onSelectPulseMode\n";
        $SetupPulsePopup::onSelectPulseMode(p0);
        if (colorInputWidget) colorInputWidget->setVisible(_pulseMode() == 0);
    }

    void colorValueChanged(cocos2d::_ccColor3B p0) {
        // std::cout << "SetupPulsePopup::colorValueChanged\n";
        $SetupPulsePopup::colorValueChanged(p0);
        if (colorInputWidget) colorInputWidget->updateLabels(true, true);
    }

    // void dtor() override {
    //     // std::cout << "SetupPulsePopup::~ColorSelectPopup\n";
    //     colorInputWidget = nullptr;
    //     $SetupPulsePopup::dtor();
    // }
} PulseSelectHook;

$apply();
