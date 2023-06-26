<div>
<p align="center">
<a href="https://www.pixilart.com/art/sustech-door-kit-logo-sr20129901307f3" >
  <img width="350" src="logo/pixil-gif-drawing.gif" alt="sustech_door_kit_logo">
</a>
  </p>  
<h1 align="center">
  南科大宿舍门锁套件
</h1>
<p align="center">
  丢掉你的校园卡
</p>
<p align="center">
人脸识别、模糊密码、电子反锁、访客密码……</a>
</p>
</div>

_This page is not available in [English](README.en.md) yet._ 

[![Repo Size](https://img.shields.io/github/repo-size/drinktoomuchsax/sustech_door_kit)](https://github.com/drinktoomuchsax/sustech_door_kit)         [![GitHub license](https://img.shields.io/github/license/drinktoomuchsax/sustech_door_kit)](https://github.com/drinktoomuchsax/sustech_door_kit/blob/main/LICENSE)       [![GitHub contributors](https://img.shields.io/github/contributors/drinktoomuchsax/sustech_door_kit)](https://github.com/drinktoomuchsax/sustech_door_kit/graphs/contributors/)                                   [![GitHub stars](https://img.shields.io/github/stars/drinktoomuchsax/sustech_door_kit?style=social)](https://github.com/drinktoomuchsax/sustech_door_kit/stargazers/)

:unlock:这个项目使用两片ESP32开发板以及一系列模块，实现了不使用校园卡打开宿舍门的功能

:star:欢迎Star

:+1:项目采用CC0协议，欢迎复刻、商用、传播


:poop:假如你想在此基础上开发新功能，看不懂屎山代码或有任何疑惑，欢迎issue

---

# :world_map:Table of Contents
- [功能](#功能)
- [原理](#原理)
- [复刻](#复刻)
  - [物料清单](#物料清单)
  - [所需工具](#所需工具)
  - [门的部分](#门的部分)
  - [门外模块（exModule）](#门外模块exmodule)
  - [门内模块（inModule）](#门内模块inmodule)
- [To Do](#to-do)



# 功能
![face recognition gif]()
![keyboard gif]()
![password define]()
![lock cant open gif]()
![random password generator]()

# 原理
拆开门发现有调试的线，详见[博客](wait-to-be-writed)

# 复刻
本项目在开始之初的设计目标之一就是尽可能低的复刻门槛，所以使用的都是现成市售的模块。虽然牺牲了一些集成度和成本，但是你不需要经历繁复地采购和焊接贴片元件，只需少许简单焊接和组装就能复刻本项目。
复刻讲解如何复刻本项目，如果对某一步的详细程度感到不满意或有任何疑惑，欢迎提交issue。

## 物料清单
## 所需工具
## 门的部分
### 1.拆门
### 2.剥线
### 3.焊接延长线
### 4.走线
### 5.装门
## 门外模块（exModule）
### 1.焊接
### 2.刷入程序
### 3.调试
### 4.组装
## 门内模块（inModule）
### 1.焊接
### 2.开发板刷入程序
#### a.更改参数
#### b.人脸注册
### 3.调试
### 4.安装
#### a.接线

# To Do
对功能有任何想法和提议，都欢迎提交issue。当然，保证不保证解决。
- [ ] 完善文档
- [ ] 接入homeassistant，让小爱同学开门
- [ ] 通信协议
- [ ] 剩下的两根线
- [ ] 更多的功能
- [ ] 更低的复刻门槛