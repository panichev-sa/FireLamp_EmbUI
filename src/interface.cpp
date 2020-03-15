/*
Copyright © 2020 Dmytro Korniienko (kDn)
JeeUI2 lib used under MIT License Copyright (c) 2019 Marsel Akhkamov

    This file is part of FireLamp_JeeUI.

    FireLamp_JeeUI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FireLamp_JeeUI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FireLamp_JeeUI.  If not, see <https://www.gnu.org/licenses/>.

  (Этот файл — часть FireLamp_JeeUI.

   FireLamp_JeeUI - свободная программа: вы можете перераспространять ее и/или
   изменять ее на условиях Стандартной общественной лицензии GNU в том виде,
   в каком она была опубликована Фондом свободного программного обеспечения;
   либо версии 3 лицензии, либо (по вашему выбору) любой более поздней
   версии.

   FireLamp_JeeUI распространяется в надежде, что она будет полезной,
   но БЕЗО ВСЯКИХ ГАРАНТИЙ; даже без неявной гарантии ТОВАРНОГО ВИДА
   или ПРИГОДНОСТИ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ. Подробнее см. в Стандартной
   общественной лицензии GNU.

   Вы должны были получить копию Стандартной общественной лицензии GNU
   вместе с этой программой. Если это не так, см.
   <https://www.gnu.org/licenses/>.)
*/

#include "main.h"
#include "effects.h"

static EFFECT *prevEffect = nullptr;
static bool isSetup = false;
static bool isTmSetup = false;

void btnTmSubmCallback()
{
#ifdef LAMP_DEBUG
    LOG.println(F("btnTmSubmCallback pressed"));
#endif
    myLamp.timeProcessor.setTimezone(jee.param(F("timezone")).c_str());
    myLamp.timeProcessor.setTime(jee.param(F("time")).c_str());

    if(myLamp.timeProcessor.getIsSyncOnline()){
        myLamp.refreshTimeManual(); // принудительное обновление времени
    }
    isTmSetup = false;
    jee.var(F("isTmSetup"), F("false"));
    jee._refresh = true;
}

void jeebuttonshandle()
{
    static unsigned long timer;
    jee.btnCallback(F("btnTmSubm"), btnTmSubmCallback);

    //публикация изменяющихся значений
    if (timer + 30*1000 > millis())
        return;
    timer = millis();
    jee.var(F("pTime"),myLamp.timeProcessor.getFormattedShortTime()); // обновить опубликованное значение
}

void parameters(){
    // создаем дефолтные параметры для нашего проекта
    jee.var(F("wifi"), F("STA")); // режим работы WiFi по умолчанию ("STA" или "AP")  (параметр в энергонезависимой памяти)
    jee.var(F("ssid"), F("")); // имя точки доступа к которой подключаемся (параметр в энергонезависимой памяти)
    jee.var(F("pass"), F("")); // пароль точки доступа к которой подключаемся (параметр в энергонезависимой памяти)

    // параметры подключения к MQTT
    jee.var(F("mqtt_int"), F("30")); // интервал отправки данных по MQTT в секундах (параметр в энергонезависимой памяти)
    
    jee.var(F("effList"), F("1"));
    jee.var(F("isSetup"), F("false"));

    jee.var(F("bright"), F("127"));
    jee.var(F("speed"), F("127"));
    jee.var(F("scale"), F("127"));
    jee.var(F("canBeSelected"), F("true"));
    jee.var(F("isFavorite"), F("true"));

    jee.var(F("ONflag"), F("true"));
    jee.var(F("MIRR_H"), F("false"));
    jee.var(F("MIRR_V"), F("false"));

    jee.var(F("GlobBRI"), F("127"));

    jee.var(F("time"), F("00:00"));
    jee.var(F("timezone"), F(""));
    jee.var(F("isTmSync"), F("true"));

    //jee.save(); // сохранить
}

void interface(){ // функция в которой мф формируем веб интерфейс
    //bool isSetup = (jee.param(F("isSetup"))==F("true"));
    
    jee.app(F(("Огненная лампа"))); // название программы (отображается в веб интерфейсе)

    // создаем меню
    jee.menu(F("Эффекты"));
    jee.menu(F("Лампа"));
    jee.menu(F("Настройки"));
    // создаем контент для каждого пункта меню

    jee.page(); // разделитель между страницами
    // Страница "Управление эффектами"

    EFFECT enEff; enEff.setNone();
    jee.checkbox(F("isSetup"),F("Setup"));
    jee.checkbox(F("ONflag"),F("Включение&nbspлампы"));
    do {
        enEff = *myLamp.effects.enumNextEffect(&enEff);
        if(enEff.eff_nb!=EFF_NONE && (enEff.canBeSelected || isSetup)){
            jee.option(String((int)enEff.eff_nb),enEff.eff_name);
        }
    } while((enEff.eff_nb!=EFF_NONE));
    jee.select(F("effList"), F("Эффект"));

    jee.range(F("bright"),1,255,1,F("Яркость"));
    jee.range(F("speed"),1,255,1,F("Скорость"));
    jee.range(F("scale"),1,255,1,F("Масштаб"));

    if(isSetup){
        jee.checkbox(F("canBeSelected"),F("В&nbspсписке&nbspвыбора"));
        jee.checkbox(F("isFavorite"),F("В&nbspсписке&nbspдемо"));  
    }

    jee.page(); // разделитель между страницами
    //Страница "Управление лампой"
    if(isTmSetup){
        jee.time(F("time"),F("Время"));
        jee.text(F("timezone"),F("Часовой пояс (http://worldtimeapi.org/api/timezone/)"));
        jee.checkbox(F("isTmSync"),F("Включить&nbspсинхронизацию"));
        jee.button(F("btnTmSubm"),F("gray"),F("Сохранить"));
    } else {
        jee.pub(F("pTime"),F("Текущее время на ESP"),F("--:--"));
        jee.var(F("pTime"),myLamp.timeProcessor.getFormattedShortTime()); // обновить опубликованное значение
    }
    jee.checkbox(F("isTmSetup"),F("Настройка&nbspвремени"));
    jee.checkbox(F("MIRR_H"),F("Инверсия&nbspH"));
    jee.checkbox(F("MIRR_V"),F("Инверсия&nbspV"));

    jee.page(); // разделитель между страницами
    // Страница "Настройки соединения"
    if(!jee.connected || jee.param(F("wifi"))==F("AP")){
        jee.formWifi(); // форма настроек Wi-Fi
        jee.formMqtt(); // форма настроек MQTT
    }
    jee.number(F("mqtt_int"), F("Интервал mqtt сек."));
    
    jee.page(); // разделитель между страницами
}

void update(){ // функция выполняется после ввода данных в веб интерфейсе. получение параметров из веб интерфейса в переменные
    // получаем данные в переменную в ОЗУ для дальнейшей работы
    bool isRefresh = false;
    EFFECT *curEff = myLamp.effects.getEffectBy((EFF_ENUM)jee.param(F("effList")).toInt());
    mqtt_int = jee.param(F("mqtt_int")).toInt();

    
    if(isTmSetup!=(jee.param(F("isTmSetup"))==F("true"))){
        isTmSetup = !isTmSetup;
        isRefresh = true;
    }

    if((jee.param(F("isSetup"))==F("true"))!=isSetup){
        isSetup = !isSetup;
        if(prevEffect!=nullptr)
            isRefresh = true;
    }

    myLamp.effects.moveBy(curEff->eff_nb);
    myLamp.restartDemoTimer();

    if(curEff->eff_nb!=EFF_NONE){
        if((curEff!=prevEffect && prevEffect!=nullptr) || isRefresh){
            jee.var(F("isFavorite"), (curEff->isFavorite?F("true"):F("false")));
            jee.var(F("canBeSelected"), (curEff->canBeSelected?F("true"):F("false")));
            //jee.var(F("bright"),String(curEff->brightness));
            jee.var(F("bright"),String(myLamp.getLampBrightness()));
            jee.var(F("speed"),String(curEff->speed));
            jee.var(F("scale"),String(curEff->scale));
            jee.var(F("ONflag"), (myLamp.isLampOn()?F("true"):F("false")));
            if(myLamp.getMode() == MODE_DEMO)
                jee.var(F("GlobBRI"), String(myLamp.getLampBrightness()));

            //LOG.printf_P("%s , передали %d %d\n",curEff->eff_name,curEff->isFavorite,curEff->canBeSelected);
            isRefresh = true;
            myLamp.startFader(true);
        } else {
            curEff->isFavorite = (jee.param(F("isFavorite"))==F("true"));
            curEff->canBeSelected = (jee.param(F("canBeSelected"))==F("true"));
            //curEff->brightness = jee.param(F("bright")).toInt();
            myLamp.setLampBrightness(jee.param(F("bright")).toInt());
            curEff->speed = jee.param(F("speed")).toInt();
            curEff->scale = jee.param(F("scale")).toInt();
            
            myLamp.setLoading(true); // перерисовать эффект
            //LOG.printf_P("%s , получили %d %d\n",curEff->eff_name,curEff->isFavorite,curEff->canBeSelected);

            if(prevEffect!=nullptr){
                if(!myLamp.effects.autoSaveConfig()){ // отложенная запись, не чаще чем однократно в 30 секунд 
                    myLamp.ConfigSaveSetup(60*1000); //через минуту сработает еще попытка записи и так до успеха
                }
            }
        }
    }
#ifdef LAMP_DEBUG
    LOG.println(F("In update..."));
#endif
    prevEffect = curEff;

    myLamp.setMIRR_H(jee.param(F("MIRR_H"))==F("true"));
    myLamp.setMIRR_V(jee.param(F("MIRR_V"))==F("true"));
    myLamp.setOnOff(jee.param(F("ONflag"))==F("true"));
    myLamp.timeProcessor.setIsSyncOnline(jee.param(F("isTmSync"))==F("true"));
    //jee.param(F("effList"))=String(0);
    jee.var(F("pTime"),myLamp.timeProcessor.getFormattedShortTime()); // обновить опубликованное значение

    jee._refresh = isRefresh; // устанавливать в самом конце!
}

void updateParm() // передача параметров в UI после нажатия сенсорной или мех. кнопки
{
#ifdef LAMP_DEBUG
    LOG.println(F("Обновляем параметры после нажатия кнопки..."));
#endif
    EFFECT *curEff = myLamp.effects.getCurrent();
    jee.var(F("bright"),String(myLamp.getLampBrightness()));
    jee.var(F("speed"),String(curEff->speed));
    jee.var(F("scale"),String(curEff->scale));
    jee.var(F("effList"),String(curEff->eff_nb));
    jee.var(F("ONflag"), (myLamp.isLampOn()?F("true"):F("false")));
    if(myLamp.getMode() == MODE_DEMO)
        jee.var(F("GlobBRI"), String(myLamp.getLampBrightness()));
    else
        myLamp.setGlobalBrightness(jee.param(F("GlobBRI")).toInt());
    myLamp.setLoading(); // обновить эффект
    prevEffect = curEff; // обновить указатель на предыдущий эффект
    jee._refresh = true;
}