# UIToolkit — JUCE 暗色主题基础 UI 组件库

一套独立的、可直接复制到任何 JUCE 项目中使用的 **基础 UI 组件集**。  
仅依赖 JUCE 框架（`<JuceHeader.h>`）和 C++ 标准库，零业务/DSP 依赖。

---

## 组件清单

### 基础原件

| 文件 | 组件 | 说明 |
|---|---|---|
| `CustomLookAndFeel.h` | `gui::Colors` / `gui::CustomLookAndFeel` | 暗色调色盘 + LookAndFeel_V4 子类 |
| `ArcKnob.h` | `gui::ArcKnob` | 270° 圆弧旋钮，渐变弧线 + 拇指点 + 居中数值 + 底部标签 |
| `SectionPanel.h` | `gui::SectionPanel` | 圆角矩形面板容器，带标题文字 |
| `ToggleSwitch.h` | `gui::ToggleSwitch` | iOS 风格滑动开关，带标签 |
| `CheckBox.h` | `gui::CheckBox` | 暗色主题勾选框，带标签 |

### 组合工具

| 文件 | 组件 | 说明 |
|---|---|---|
| `KnobStrip.h` | `gui::KnobDescriptor` / `gui::KnobStrip` | 声明式一行旋钮管理器，自动 APVTS 绑定 + 等宽布局 |
| `SectionBase.h` | `gui::SectionBase` | Section 基类 = SectionPanel + KnobStrip + 可扩展内容区 |

### 总头文件

```cpp
#include "UIToolkit/UIToolkit.h"  // 一次引入所有组件
```

---

## 如何在其他项目中使用

### 1. 复制

将整个 `UIToolkit/` 目录复制到你的 JUCE 项目 `Source/` 下：

```
YourProject/
  Source/
    UIToolkit/        ← 复制整个目录
    PluginProcessor.h
    PluginEditor.h
```

### 2. 注册到构建系统

**Projucer**: 在 Source Group 中 Add Existing Files，选中 `UIToolkit/` 下的所有 `.h` 文件。

**CMake**:

```cmake
file(GLOB UITOOLKIT_HEADERS Source/UIToolkit/*.h)
target_sources(YourPlugin PRIVATE ${UITOOLKIT_HEADERS})
```

### 3. 设置 LookAndFeel

```cpp
#include "UIToolkit/CustomLookAndFeel.h"

class MyEditor : public juce::AudioProcessorEditor
{
public:
    MyEditor(MyProcessor& p) : AudioProcessorEditor(&p)
    {
        setLookAndFeel(&lnf);
    }
    ~MyEditor() override { setLookAndFeel(nullptr); }
private:
    gui::CustomLookAndFeel lnf;
};
```

---

## 使用示例

### ArcKnob — 圆弧旋钮

```cpp
#include "UIToolkit/ArcKnob.h"

gui::ArcKnob cutoffKnob{ "Cutoff", "Hz" };
addAndMakeVisible(cutoffKnob);

// 绑定 APVTS 参数
auto attach = std::make_unique<
    juce::AudioProcessorValueTreeState::SliderAttachment>(
    apvts, "cutoff", cutoffKnob.getSlider());
```

### ToggleSwitch — 滑动开关

```cpp
#include "UIToolkit/ToggleSwitch.h"

gui::ToggleSwitch bypassSwitch{ "Bypass" };
addAndMakeVisible(bypassSwitch);

// 绑定 APVTS boolean 参数
auto attach = std::make_unique<
    juce::AudioProcessorValueTreeState::ButtonAttachment>(
    apvts, "bypass", bypassSwitch.getButton());
```

### CheckBox — 勾选框

```cpp
#include "UIToolkit/CheckBox.h"

gui::CheckBox oversampling{ "2x Oversampling" };
addAndMakeVisible(oversampling);

// 绑定 APVTS boolean 参数
auto attach = std::make_unique<
    juce::AudioProcessorValueTreeState::ButtonAttachment>(
    apvts, "oversample", oversampling.getButton());
```

### SectionPanel — 分区面板

```cpp
#include "UIToolkit/SectionPanel.h"

gui::SectionPanel panel{ "FILTER" };
addAndMakeVisible(panel);

// 在 resized() 中
panel.setBounds(getLocalBounds());
auto content = panel.getContentArea();  // 标题下方的可用区域
myKnob.setBounds(content);
```

### KnobStrip — 一行旋钮

```cpp
#include "UIToolkit/KnobStrip.h"

gui::KnobStrip strip;
strip.init(apvts, {
    { "Attack",  "ms", "envAttack" },
    { "Decay",   "ms", "envDecay" },
    { "Sustain", "",   "envSustain" },
    { "Release", "ms", "envRelease" }
});
addAndMakeVisible(strip);

// resized() 中
strip.setBounds(area);  // 旋钮自动等宽排列
```

### SectionBase — 继承创建自定义 Section

```cpp
#include "UIToolkit/SectionBase.h"

class FilterSection : public gui::SectionBase
{
public:
    FilterSection(juce::AudioProcessorValueTreeState& apvts)
        : SectionBase("FILTER", apvts, {
              { "Cutoff", "Hz", "filterCutoff" },
              { "Reso",   "",   "filterReso" }
          })
    {
        addAndMakeVisible(myDisplay);
    }
protected:
    void resizeContent(juce::Rectangle<int> area) override
    {
        myDisplay.setBounds(area);
    }
private:
    MyFilterDisplay myDisplay;
};
```

纯旋钮 Section（无额外内容）传 `knobRowHeight = 0`：

```cpp
class GainSection : public gui::SectionBase
{
public:
    GainSection(juce::AudioProcessorValueTreeState& apvts)
        : SectionBase("OUTPUT", apvts, {
              { "Gain", "dB", "gain" },
              { "Pan",  "",   "pan" }
          }, 0)  // 旋钮填满整个内容区
    {}
};
```

---

## 自定义配色

所有颜色集中定义在 `gui::Colors` 命名空间（`CustomLookAndFeel.h`）：

```cpp
namespace gui::Colors
{
    static const juce::Colour background     { 0xff1A1A2E };  // 全局背景
    static const juce::Colour panelBackground{ 0xff16213E };  // 面板背景
    static const juce::Colour panelBorder    { 0xff0F3460 };  // 面板边框
    static const juce::Colour accent         { 0xffE94560 };  // 强调色（旋钮弧线、标题、开关等）
    static const juce::Colour accentDark     { 0xffA83279 };  // 渐变暗端
    static const juce::Colour textBright     { 0xffEEEEEE };  // 亮文字
    static const juce::Colour textDim        { 0xff8899AA };  // 暗文字
    static const juce::Colour knobBackground { 0xff0A0E1A };  // 旋钮圆盘背景
    static const juce::Colour knobArcTrack   { 0xff2A2E4A };  // 弧线轨道
    static const juce::Colour knobArcActive  { 0xffE94560 };  // 弧线激活色
    static const juce::Colour knobThumb      { 0xffFFFFFF };  // 拇指点
    // ...
}
```

修改这些值即可全局换肤。

---

## 依赖关系

```
CustomLookAndFeel.h        ← 基础：Colors + LookAndFeel
├── ArcKnob.h
├── SectionPanel.h
├── ToggleSwitch.h
└── CheckBox.h

ArcKnob.h
└── KnobStrip.h

SectionPanel.h + KnobStrip.h
└── SectionBase.h

UIToolkit.h                ← 总头文件，包含以上全部
```

## 要求

- **JUCE 7.x / 8.x**（使用 `juce::FontOptions`）
- **C++17** 或更高
