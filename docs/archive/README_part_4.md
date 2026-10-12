# Полная документация — Часть 4

> Продолжение. Начало — в README_part_1.md, README_part_2.md, README_part_3.md

---

## СОДЕРЖАНИЕ ЧАСТИ 4

27. Схемы
28. Ошибки, которые были исправлены
29. Открытые темы
30. Завершение

---

## ЧАСТЬ 27. СХЕМЫ

### 27.1. Схема иерархии

СЛУЖБА ГЛАВНОГО ТЕХНОЛОГА (Division of Chief Technologist)
│
ДИРЕКЦИЯ ПО ИНДУСТРИАЛИЗАЦИИ ПРОЕКТОВ (Project Industrialization Directorate)
│
УПП — УПРАВЛЕНИЕ ПРОИЗВОДСТВА ПРОТОТИПОВ (Prototype Manufacturing Department)
│
├── Цех изготовления кузова и оснастки (Body & Tooling Manufacturing Shop)
│ ├── Участок жестянщиков (Sheet Metal Workers Area)
│ ├── Участок сварки (Welding Area)
│ ├── Участок разметки заготовок (Marking Area)
│ ├── Участок формовки (Sand Core Forming Area)
│ └── Участок механической обработки по дереву (Wood Machining Area)
│
├── Цех сборки прототипов (Prototype Assembly Shop)
│ ├── Участок сборки автомобилей (Vehicle Assembly Area)
│ ├── Участок сборки ДВС и агрегатирования (Engine Assembly & Aggregation Area)
│ └── Участок механической обработки по металлу (Metal Machining Area)
│
├── КТО (Process Engineering Section)
│ ├── Бюро оснастки (Tooling Bureau)
│ ├── Бюро управляющих программ (NC Programming Bureau)
│ ├── Бюро изготовления деталей (Parts Manufacturing Bureau)
│ ├── Бюро сборки (Assembly Bureau)
│ └── Группа проработчиков составов (Design Review Group)
│
├── Отдел экономистов (Cost Engineering Section)
│ └── Бюро нормативов и затрат (Norm & Cost Bureau)
│
├── Отдел планирования (Production Planning Section)
│ └── Бюро плановиков (Planning Bureau)
│
├── Отдел логистики (Internal Logistics Section)
│ ├── Бюро внутренней логистики (Internal Logistics Bureau)
│ ├── Бюро обеспечения сборки (Assembly Support Bureau)
│ └── Бюро обеспечения производства (Production Support Bureau)
│
├── Отдел качества (Quality Control Section)
│ ├── Бюро контроля качества сборки (Assembly Quality Control Bureau)
│ ├── Бюро AVES (AVES Bureau)
│ └── Участок замеров (Measurement Area)
│
└── Инструментальный участок / Кладовая оснастки (Tool Store)

СЛУЖБА ТЕХНИЧЕСКОГО СОПРОВОЖДЕНИЯ (Technical Support Division)
│
Отдел обеспечения инжиниринга (Engineering Supply Section)
├── Бюро оригинальных (покупных) КИ (Prototype Component Supply Bureau)
└── Бюро серийных КИ (Serial Component Supply Bureau)

ДПЗ — ДИРЕКЦИЯ ПО ЗАКУПКАМ (Procurement Directorate)
│
Серийные контракты. К прототипам — никакого отношения.
│
Структура: НЕИЗВЕСТНА.

ДИС — ДИРЕКЦИЯ ПО ИНФОРМАЦИОННЫМ СИСТЕМАМ (IT Directorate)
│
Структура: НЕИЗВЕСТНА.

СЕРИЙНОЕ ПРОИЗВОДСТВО (Serial Production)
│
Внешнее для УПП. Почти не касается ERP.

ГЛАВНАЯ БУХГАЛТЕРИЯ (General Accounting)
│
Функция: оплата PO.

КОМАНДА ПРОЕКТА (Project Team)
├── Главный инженер (Chief Engineer)
├── прото-РП (Prototype Project Manager)
├── Руководители проектов (Project Managers)
├── Экономист проекта (Project Economist)
├── Инженер по планированию продукта (Product Planning Engineer)
├── Специалист по валидации (Validation Specialist)
└── Инженер по качеству (Quality Engineer)


### 27.2. Mermaid — административная структура УПП

```mermaid
graph TD
    DCT[Division of Chief Technologist] --> PID[Project Industrialization Directorate]
    PID --> PMD[Prototype Manufacturing Department]

    PMD --> SH1[Body & Tooling Manufacturing Shop]
    SH1 --> SH1_A1[Sheet Metal Workers Area]
    SH1 --> SH1_A2[Welding Area]
    SH1 --> SH1_A3[Marking Area]
    SH1 --> SH1_A4[Sand Core Forming Area]
    SH1 --> SH1_A5[Wood Machining Area]

    PMD --> SH2[Prototype Assembly Shop]
    SH2 --> SH2_A1[Vehicle Assembly Area]
    SH2 --> SH2_A2[Engine Assembly & Aggregation Area]
    SH2 --> SH2_A3[Metal Machining Area]

    PMD --> KTO[Process Engineering Section]
    KTO --> KTO_B1[Tooling Bureau]
    KTO --> KTO_B2[NC Programming Bureau]
    KTO --> KTO_B3[Parts Manufacturing Bureau]
    KTO --> KTO_B4[Assembly Bureau]
    KTO --> KTO_G1[Design Review Group]

    PMD --> ECON[Cost Engineering Section]
    ECON --> ECON_B1[Norm & Cost Bureau]

    PMD --> PLAN[Production Planning Section]
    PLAN --> PLAN_B[Planning Bureau]

    PMD --> LOG[Internal Logistics Section]
    LOG --> LOG_B1[Internal Logistics Bureau]
    LOG --> LOG_B2[Assembly Support Bureau]
    LOG --> LOG_B3[Production Support Bureau]

    PMD --> QC[Quality Control Section]
    QC --> QC_B1[Assembly Quality Control Bureau]
    QC --> QC_B2[AVES Bureau]
    QC --> QC_A1[Measurement Area]

    PMD --> TOOL[Tool Store]

27.3. Mermaid — полный процесс

flowchart TD
    START([Запрос от проекта]) --> PPM[прото-РП]

    PPM --> PAR{Параллельно}
    PAR --> REVIEW[Группа проработчиков]
    PAR --> TECH[Технологи]

    REVIEW -->|итерации| REVIEW
    REVIEW -->|валидирован| COND
    TECH -->|подтверждено| COND
    PLAN_B[Плановик] --> COND

    COND{Условия выпуска ТЗ}
    COND -->|выполнены| TZ[Выпуск ТЗ]
    TZ --> APPROVE[Согласование]
    APPROVE -->|ТЗ действительно| TRIGGER{ТЗ — триггер}

    TRIGGER --> TD[Разработка ТД]
    TRIGGER --> REQ[Заявки на КИ]
    TRIGGER --> SERIAL_ORDER[Наряд-заказ в серийное производство]

    TD --> DISPATCH[Плановик распределяет ТД]
    DISPATCH --> PROD[Изготовление в цехах УПП]

    REQ --> LOG_B2[Бюро обеспечения сборки]
    LOG_B2 --> SUP_B2[Бюро серийных КИ СТС]
    LOG_B2 --> SUP_B1[Бюро оригинальных КИ СТС]

    SUP_B2 -->|накладные| WAREHOUSE[Склад УПП]
    SUP_B1 -->|запрос| SUPPLIER[Поставщик]
    SUPPLIER -->|PO| PO[PO]
    PO --> APPROVE2[Согласование PO]
    APPROVE2 -->|согласовано| SIGN[Подписание]
    SIGN --> PAY[Главная бухгалтерия]
    PAY --> MANUF[Изготовление]
    MANUF --> DELIVER[Логисты СТС]
    DELIVER --> WAREHOUSE

    SERIAL_ORDER --> SERIAL[Серийное производство]
    PPM -->|договорённость| SERIAL_PM[РП серийного производства]
    SERIAL_PM --> SERIAL
    LOG_B1[Логисты УПП] -->|накладные| SERIAL
    SERIAL -->|обработанные заготовки| LOG_B1
    LOG_B1 --> WAREHOUSE
    SERIAL --> ACT_WORK[Акт выполненных работ]

    PROD --> QC[Выходной контроль]
    QC --> WAREHOUSE

    WAREHOUSE -->|все КИ в наличии| ASSEMBLY[Команда на сборку]
    ASSEMBLY --> TECH_ASSEMBLY[Бюро сборки — техпроцесс]
    TECH_ASSEMBLY --> WAREHOUSE2[Склад комплектует]
    WAREHOUSE2 --> MASTER[Мастер участка]
    MASTER --> WORKER[Рабочий]
    WORKER --> QC2[Контролёр]
    QC2 --> ACT[Акт об изготовлении]
    ACT --> PPM_NOTIFY[Информирование прото-РП]
    PPM_NOTIFY --> TRANSFER[Передача заказчику]
    TRANSFER --> LOG_DOC[Логист УПП]
    LOG_DOC --> SIGN2[Подписание]
    SIGN2 --> END([Прототип передан])

    PPM --> REPORT[Отчётность раз в месяц]
    ECON --> REPORT
    REPORT --> TEAM_OUT[Проект]
    REPORT --> MGMT[Руководство]

27.4. Mermaid — потоки взаимодействия

flowchart LR
    subgraph CHTECH[Служба главного технолога]
        subgraph PID[ДИП]
            subgraph PMD[УПП]
                PLAN[Production Planning Section]
                LOG_B2[Assembly Support Bureau]
                LOG_B1[Internal Logistics Bureau]
                SHOP[Prototype Shops]
                KTO[Process Engineering Section]
                ECON[Cost Engineering Section]
                QC[Quality Control Section]
            end
        end
    end

    subgraph STS[СТС]
        SUP_B1[Prototype Component Supply Bureau]
        SUP_B2[Serial Component Supply Bureau]
    end

    subgraph TEAM[Команда проекта]
        PPM[прото-РП]
        PE[Project Economist]
        CE[Chief Engineer]
    end

    SUPPLIERS[Поставщики]
    SERIAL[Серийное производство]
    SERIAL_WAREHOUSE[Серийные склады]
    GA[Главная бухгалтерия]

    PLAN -->|план| SHOP
    PLAN -->|загрузка| KTO

    SHOP -->|потребность| LOG_B2
    LOG_B2 -->|запрос покупных| SUP_B1
    LOG_B2 -->|запрос серийных| SUP_B2

    SUP_B1 -->|PR| SUPPLIERS
    SUPPLIERS -->|поставка| SUP_B1
    SUP_B1 -->|передача| LOG_B1

    SERIAL_WAREHOUSE -->|накладные| SUP_B2
    SUP_B2 -->|перемещения| LOG_B1

    LOG_B1 -->|заготовки и КИ| SHOP

    ECON -->|учёт| SHOP
    ECON -->|наряд-заказ| SERIAL
    PPM -->|договорённость| SERIAL

    KTO -->|техпроцессы| SHOP
    QC -->|контроль| SHOP

    GA -->|оплата PO| SUP_B1

    PPM -->|отчётность| CE
    PPM -->|отчётность| PE
    ECON -->|отчётность| CE

27.5. Mermaid — типы запросов

flowchart TD
    REQ[Запрос в УПП] --> TYPE{Тип запроса}

    TYPE -->|1| T1[Изготовление с 0]
    TYPE -->|2| T2[Ретрофит]
    TYPE -->|3| T3[BUK]
    TYPE -->|4| T4[Доработка]
    TYPE -->|5| T5[Поддержка производства]
    TYPE -->|6| T6[Мул MULE]

    T3 --> T3_1[Kit]
    T3 --> T3_2[Body]
    T3 --> T3_3[Unit]

    T6 --> T6_1[Серийный носитель]
    T6 --> T6_2[Переоборудование в УПП]
    T6 --> T6_3[Испытания]
    T6_3 --> T6_4{Повторное переоборудование?}
    T6_4 -->|да| T2
    T6_4 -->|нет| T6_5[Утилизация]

27.6. Mermaid — цикл мула и ретрофита

flowchart LR
    SERIAL[Серийный автомобиль] -->|переоборудование| MULE[МУЛ]
    MULE -->|испытания| TEST1[Испытания]
    TEST1 -->|ретрофит| RETROFIT[Ретрофит]
    RETROFIT -->|испытания| TEST2[Испытания]
    TEST2 -->|утилизация| UTIL[Утилизация]

27.7. Mermaid — процесс с кокилем

flowchart TD
    START[Потребность в опытных отливках] --> PPM[прото-РП управляет вручную]
    PPM -->|договорённость| SERIAL_MET[Серийное металлургическое производство]
    SERIAL_MET -->|кокиль| TRANSFER1[Перевозка в УПП]
    TRANSFER1 --> AREA[Участок формовки]
    AREA -->|замена элементов| MOD1[Вставки, полуформы]
    MOD1 -->|геометрия| MOD2[Мастика]
    MOD2 -->|нестандартный кокиль| TRANSFER2[Перевозка в металлургическое производство]
    TRANSFER2 --> CAST[Изготовление опытных отливок]
    CAST -->|отливки| TRANSFER3[Перевозка в УПП]
    CAST -->|кокиль| TRANSFER4[Возврат в УПП]
    TRANSFER4 --> RESTORE[Восстановление]
    RESTORE --> TRANSFER5[Возврат в серийное производство]
    TRANSFER3 --> PROCESS[Дальнейшая обработка]
    PPM -->|оформление| ORDER[Наряд-заказ]

27.8. Mermaid — фазы проекта

flowchart LR
    UP[Upstream] --> DEV[Development]
    DEV --> IND[Industrialization]

    UP -->|не все проекты| UP_NOTE[Market research; Потребительские свойства; Дизайн; Глиняные макеты; Аэродинамика]
    DEV -->|прототипы| DEV_NOTE[КД; Прототипы; Испытания; Итерации]
    IND -->|без прототипов| IND_NOTE[Серийные договоры; Оснастка; ТД; Обучение; AVES]

27.9. Mermaid — команда проекта

graph TD
    CE[Chief Engineer] --> PPM[Prototype Project Manager]
    CE --> PM[Project Managers]
    CE --> PE[Project Economist]
    CE --> PPE[Product Planning Engineer]
    CE --> VS[Validation Specialist]
    CE --> QE[Quality Engineer]

    PPE --> MS[Master Schedule]
    VS --> VP[Validation Plan]

ЧАСТЬ 28. ОШИБКИ, КОТОРЫЕ БЫЛИ ИСПРАВЛЕНЫ
№	Ошибка	Исправление
1	УПП — дирекция	УПП — управление (Department)
2	Отдел обеспечения инжиниринга — в УПП	В СТС (ранее СТО)
3	ДПЗ приписаны бюро рамочных договоров и операционных закупок	Выдумано. В диалоге их не было
4	ДПЗ участвует в обеспечении прототипами	Не участвует. Вообще.
5	«Внешнее подразделение закупок»	Не существует
6	Склад «формирует комплект»	Склад комплектует комплект КИ в соответствии с комплектовочной ведомостью и передаёт мастеру
7	Мастер формирует комплект	Мастер получает комплект от склада
8	Транспортный участок	УБРАН. Выдуман
9	Бюро технологов в КТО	Убрано. Вместо него — 4 бюро и 1 группа
10	Бюро проработки составов	Группа проработчиков составов
11	Бюро изготовления оснастки	Бюро оснастки
12	Бюро межоперационных перемещений	Бюро внутренней логистики
13	Бюро запросов на КИ	Бюро обеспечения сборки
14	СТО	СТС — Служба технического сопровождения
15	Дирекция без названия	ДИП — Дирекция по индустриализации проектов
16	Матричное подчинение прото-РП к УПП, СТС, ГБ, серийному производству	Вопиющая ахинея. Убрано
17	AVES — процедура сбора замечаний	Уточнено: при сборке опытной партии на серийной оснастке, перед вехой VC, в фазе Industrialization
18	Количество мулов	4–10 на первый лот (и на проект)
19	Лоты	2–3 в автомобильном проекте. Первый — почти всегда мулы
20	Заместитель главного инженера в команде	Убран. Он по сути другой главный инженер проекта, ведёт свой проект
ЧАСТЬ 29. ОТКРЫТЫЕ ТЕМЫ

    Что во втором и третьем лотах?

    Подразделение, занимающееся испытаниями — как называется?

    Передача мула в испытания — по какому документу?

    ДПЗ — структура.

    ДИС — структура.

    СТС — что ещё.

    ДИП — что ещё.

    СГТ — что ещё.

    Статусы (прототипов, лотов, ТЗ, PO, заявок).

    Чеклисты.

    KPI, метрики.

    Компетенции.

    Риски.

    Кокиль — источник.

    Мастерграфик — отдельная тема.

    Бюджет — отдельная тема.

    Вехи — тема Мастерграфиков.

ЧАСТЬ 30. ЗАВЕРШЕНИЕ

Это полная документация на основе всего диалога.

    Без упрощений.

    Без выжимок.

    Без потерь.

    С фиксацией контекста.

    С фиксацией открытых тем.

    С фиксацией ограничений.

    С фиксацией ошибок, которые были исправлены.
