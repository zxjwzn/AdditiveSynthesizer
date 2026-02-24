# GUI Toolkit — 暗色主题合成器 UI 组件库

本目录下的所有文件构成一套 **独立的、可复用的 JUCE GUI 组件库**。  
它仅依赖 JUCE 框架本身（`<JuceHeader.h>`）和 C++ 标准库，**不依赖任何 DSP / 业务逻辑代码**，可以直接复制到任何 JUCE 项目中使用。

---

## 文件清单

| 文件 | 职责 | 层级 |
|---|---|---|
| `CustomLookAndFeel.h` | 暗色主题调色盘 (`Colors`) + LookAndFeel_V4 子类 | 基础 |
| `ArcKnob.h` | 圆弧旋钮（270° 弧线 + 渐变 + 值显示 + 标签） | 基础组件 |
| `SectionPanel.h` | 带标题的圆角面板容器 | 基础组件 |
| `KnobStrip.h` | 一行旋钮的声明式管理器（自动 APVTS 绑定 + 等宽布局） | 组合组件 |
| `SectionBase.h` | Section 基类（面板 + 旋钮行 + 可扩展内容区） | 组合组件 |
| `WaveformDisplay.h` | 实时波形可视化 | 可视化 |
| `ADSRDisplay.h` | ADSR 包络曲线可视化 | 可视化 |
| `SpectrumDisplay.h` | 频谱柱状图 + 滤波器截止线可视化 | 可视化 |
| `OscillatorSection.h` | 振荡器 Section（旋钮行 + 波形显示） | 业务 Section |
| `SpectralFilterSection.h` | 频谱滤波器 Section（旋钮行 + 频谱显示 + 文件加载） | 业务 Section |
| `EnvelopeSection.h` | ADSR 包络 Section（旋钮行 + 包络显示） | 业务 Section |
| `UnisonOutputSection.h` | 齐奏与输出 Section（纯旋钮行） | 业务 Section |

### 依赖关系图

```
CustomLookAndFeel.h          ← 所有组件依赖此文件的 Colors + LookAndFeel
├── ArcKnob.h
├── SectionPanel.h
├── WaveformDisplay.h
├── ADSRDisplay.h
└── SpectrumDisplay.h         (定义了 SpectrumData 结构体)

ArcKnob.h
└── KnobStrip.h               (管理多个 ArcKnob + APVTS 绑定)

SectionPanel.h + KnobStrip.h
└── SectionBase.h              (Section 基类)

SectionBase.h
├── OscillatorSection.h        + WaveformDisplay.h
├── SpectralFilterSection.h    + SpectrumDisplay.h
├── EnvelopeSection.h          + ADSRDisplay.h
└── UnisonOutputSection.h
```

---

## 如何在其他 JUCE 项目中使用

### 1. 复制文件

将整个 `GUI/` 目录复制到你的项目 `Source/` 下：

```
YourProject/
  Source/
    GUI/               ← 复制整个目录
    PluginProcessor.h
    PluginEditor.h
    ...
```

### 2. 在构建系统中注册

**Projucer**：在 Source Group 中 Add Existing Files，选中所有 `.h` 文件。

**CMake**：
```cmake
target_sources(YourPlugin PRIVATE
    Source/GUI/CustomLookAndFeel.h
    Source/GUI/ArcKnob.h
    Source/GUI/SectionPanel.h
    Source/GUI/KnobStrip.h
    Source/GUI/SectionBase.h
    Source/GUI/WaveformDisplay.h
    Source/GUI/ADSRDisplay.h
    Source/GUI/SpectrumDisplay.h
    # ... 以及你需要的 Section 文件
)
```

### 3. 应用 LookAndFeel

在 Editor 构造函数中设置全局主题：

```cpp
#include "GUI/CustomLookAndFeel.h"

class MyEditor : public juce::AudioProcessorEditor
{
public:
    MyEditor(MyProcessor& p) : AudioProcessorEditor(&p)
    {
        setLookAndFeel(&lnf);
        // ...
    }
    ~MyEditor() override { setLookAndFeel(nullptr); }

private:
    gui::CustomLookAndFeel lnf;
};
```

---

## 使用示例

### 示例 1：直接使用 ArcKnob

```cpp
#include "GUI/ArcKnob.h"

// 在你的组件中
gui::ArcKnob myKnob{ "Cutoff", "Hz" };
addAndMakeVisible(myKnob);

// 绑定到 APVTS 参数
auto attachment = std::make_unique<
    juce::AudioProcessorValueTreeState::SliderAttachment>(
    apvts, "cutoffParam", myKnob.getSlider());
```

### 示例 2：使用 KnobStrip 管理一行旋钮

```cpp
#include "GUI/KnobStrip.h"

gui::KnobStrip strip;
strip.init(apvts, {
    { "Attack",  "s",  "envAttack" },
    { "Decay",   "s",  "envDecay" },
    { "Sustain", "",   "envSustain" },
    { "Release", "s",  "envRelease" }
});
addAndMakeVisible(strip);

// resized() 中
strip.setBounds(knobArea);  // 旋钮自动等宽排列
```

### 示例 3：继承 SectionBase 创建自定义 Section

```cpp
#include "GUI/SectionBase.h"
#include "GUI/ADSRDisplay.h"

class MyEnvelopeSection : public gui::SectionBase
{
public:
    MyEnvelopeSection(juce::AudioProcessorValueTreeState& apvts)
        : SectionBase("MY ENVELOPE", apvts, {
              { "Attack",  "ms", "attack" },
              { "Release", "ms", "release" }
          })
        // knobRowHeight 默认 80px
    {
        addAndMakeVisible(display);
    }

protected:
    void resizeContent(juce::Rectangle<int> area) override
    {
        display.setBounds(area);
    }

private:
    gui::ADSRDisplay display;
};
```

### 示例 4：纯旋钮 Section（无额外内容）

```cpp
class GainSection : public gui::SectionBase
{
public:
    GainSection(juce::AudioProcessorValueTreeState& apvts)
        : SectionBase("OUTPUT", apvts, {
              { "Gain",  "dB", "masterGain" },
              { "Pan",   "",   "pan" }
          }, 0)  // knobRowHeight=0 → 旋钮填满整个内容区
    {}
};
```

### 示例 5：使用 SpectrumDisplay（外部数据注入）

```cpp
gui::SpectrumDisplay specDisplay;

// 每帧更新（在 timerCallback 中）
gui::SpectrumData data;
data.amplitudes = myFFTResult.data();
data.count      = static_cast<int>(myFFTResult.size());
specDisplay.setSpectrumData(data);
specDisplay.setFilterParams(cutoffValue, boostValue, stretchValue);
```

### 示例 6：SpectralFilterSection 的文件加载回调

```cpp
// 通过 std::function 回调解耦，不依赖任何具体 DSP 类型
gui::SpectralFilterSection section(apvts,
    [&myAnalyzer](const juce::File& file) {
        return myAnalyzer.loadFile(file);
    });
```

---

## 架构设计要点

### 零 DSP 依赖

所有 GUI 组件的外部数据输入使用以下机制，**不引用任何 DSP 头文件**：

| 数据流 | 机制 |
|---|---|
| 参数绑定 | `juce::AudioProcessorValueTreeState` + `SliderAttachment` |
| 波形可视化 | `const juce::AudioBuffer<float>*` 指针 |
| 频谱可视化 | `gui::SpectrumData { const float*, int }` 值类型 |
| 文件加载 | `gui::FileLoadCallback = std::function<bool(const juce::File&)>` |

### 颜色主题

所有颜色定义在 `gui::Colors` 命名空间中。若需自定义配色，修改 `CustomLookAndFeel.h` 中的 `Colors` 常量即可全局生效。

### SectionBase 模式

`SectionBase` 封装了 Section 组件的通用模式：

```
┌─────────── SectionPanel (标题面板) ─────────┐
│ SECTION TITLE                               │
│ ┌─────────── KnobStrip (旋钮行) ──────────┐ │
│ │  [Knob1]  [Knob2]  [Knob3]  [Knob4]    │ │
│ └─────────────────────────────────────────┘ │
│ ┌── resizeContent() → 自定义内容区 ──────┐  │
│ │                                         │  │
│ │  (Display / Button / 自由布局)           │  │
│ │                                         │  │
│ └─────────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

子类只需提供：
1. 标题字符串
2. `KnobDescriptor` 数组（声明式旋钮定义）
3. 可选：覆写 `resizeContent()` 布局额外组件

---

## 要求

- **JUCE 7.x / 8.x**（使用 `juce::FontOptions`，需要较新的 JUCE 版本）
- **C++17** 或更高
