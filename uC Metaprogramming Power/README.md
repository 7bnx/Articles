# Включаем периферию контроллера за 1 такт или магия 500 строк кода

####
<img src="https://habrastorage.org/webt/m6/5v/6n/m65v6nk2ktzzuhm0wofwz8bff4o.png" />

Как часто, при разработке прошивки для микроконтроллера, во время отладки, когда байтики не бегают по UART, Вы восклицаете: «Ааа, точно! Забыл включить тактирование!». Или, при смене ножки светодиода, забывали «подать питание» на новый порт? Думаю, что довольно часто. Я, по крайней мере, -  уж точно. 

На первый взгляд может показаться, что управление тактированием периферии тривиально: записал 1 - включил,  0 - выключил. 

Но просто, -  не всегда оказывается эффективно…
<cut />  
<h2>Постановка задачи</h2>

##
Прежде, чем писать код, необходимо определить критерии, по которым возможна его оценка. В случае с системой тактирования периферии контроллера список может выглядеть следующим образом:
  
<ul>
  <li>Во встраиваемых системах, один из самых главных критериев — это минимально-возможный результирующий код, исполняемый за минимальное время </li>
  <li>Легкая масштабируемость. Добавление или изменение в проекте какой-либо периферии не должно сопровождаться code review всех исходников, чтобы удалить строчки включения/отключения тактирования </li>
  <li>Пользователь должен быть лишен возможности совершить ошибку, либо, по крайней мере эта возможность должна быть сведена к минимуму</li>
  <li>Нет необходимости работы с отдельными битами и регистрами</li>
  <li>Удобство и однообразность использования независимо от микроконтроллера </li>
  <li>Помимо основных возможностей включения и выключения тактирования периферии необходим расширенный функционал (о нем речь пойдет далее) </li>
</ul>
После выяснения критериев оценки, поставим конкретную задачу, попутно определив условия и «окружение» для реализации:

Компилятор: <i>GCC 10.1.1 + Make</i>  
Язык: <i>C++17</i>  
Среда: <i>Visual Studio Code</i>  
Контроллер: <i>stm32f103c8t6 (cortex-m3)</i>  
Задача: <i>включение тактирования SPI2, USART1 </abbr> (оба интерфейса с использованием DMA)</i>

Выбор данного контроллера обусловлен, естественно, его распространённостью, особенно, благодаря одному из китайских народных промыслов – производству плат Blue Pill.
####
<img src="https://habrastorage.org/webt/dy/kr/tr/dykrtr_rya_ezallc1rlr01dpak.png" />

####
С точки зрения идеологии, совершенно неважно, какой именно контроллер выбран: stmf1, stmf4 или lpc, т.к. работа с системой тактирования периферии сводится лишь к записи в определенный бит либо 0 для выключения, либо 1 для включения. 

В stm32f103c8t6 имеется 3 регистра, которые ответственны за включение тактирования периферии: AHBENR, APB1ENR, APB2ENR. 
Аппаратные интерфейсы передачи данных SPI2 и USART1 выбраны неслучайно, потому что для их полноценного функционирования необходимо включить биты тактирования, расположенные во всех перечисленных регистрах – биты самих интерфейсов, DMA1, а также биты портов ввода-вывода (GPIOB для SPI2 и GPIOA для USART1).
<img src="https://habrastorage.org/webt/3n/x3/cy/3nx3cyivj--xfv3mtaujrdvebmy.png" /> <img src="https://habrastorage.org/webt/zf/zs/kq/zfzskqrorhzx8nx3iknpk5ivrjy.png" />
<img src="https://habrastorage.org/webt/g1/ve/hn/g1vehnaxqiq9aoebwrmw9qqsk5q.png" />

Следует отметить, что для оптимальной работы с тактированием, необходимо учитывать – AHBENR содержит разделяемый ресурс, используемые для функционирования как SPI2, так и USART1. То есть, отключение DMA сразу приведет к неработоспособности обоих интерфейсов, вместе с тем, КПД повторного включения будет даже не нулевым, а отрицательным, ведь эта операция займет память программ и приведет к дополнительному расходу тактов на чтение-модификацию-запись volatile регистра.

Разобравшись с целями, условиями и особенностями задачи, перейдем к поиску решений.
 
<h2>Основные подходы</h2>

##
В этом разделе собраны типовые способы включения тактирования периферии, которые мне встречались и, наверняка, Вы их также видели и/или используете.    От более простых, - реализуемых на C, до fold expression из C++17. Рассмотрены присущие им достоинства и недостатки.
 
Если Вы хотите перейти непосредственно к метапрограммированию, то этот раздел можно пропустить и перейти к [следующему](#Расширяем-функционал).

<h3>Прямая запись в регистры</h3>

##
Классический способ, «доступный из коробки» и для С и для C++. Вендор, чаще всего, представляет заголовочные файлы для контроллера, в которых задефайнены все регистры и их биты, что дает возможность сразу начать работу с периферией:

```cpp 
int main(){
  RCC->AHBENR  |= RCC_AHBENR_DMA1EN;
  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN
               |  RCC_APB2ENR_IOPBEN
               |  RCC_APB2ENR_USART1EN;
  RCC->APB2ENR |= RCC_APB1ENR_SPI2EN;
…
}

```
####
<details> 
  <summary>Листинг</summary>
<p>

```s
    // AHBENR(Включение DMA1)
  ldr     r3, .L3
  ldr     r2, [r3, #20]
  orr     r2, r2, #1
  str     r2, [r3, #20]
    // APB2ENR(Включение GPIOA, GPIOB, USART1)
  ldr     r2, [r3, #24]
  orr     r2, r2, #16384
  orr     r2, r2, #12
  str     r2, [r3, #24]
    // APB1ENR(Включение SPI2)
  ldr     r2, [r3, #28]
  orr     r2, r2, #16384
  str     r2, [r3, #28]
```
</p>
</details>

####
Размер кода: 36 байт. <a href="https://godbolt.org/z/v9E4nc">Посмотреть</a>

<b>Плюсы:</b>
<ul>
	<li>Минимальный размер кода и скорость выполнения</li>
<li>Самый простой и очевидный способ </li>
</ul>
<b>Минусы:</b>
<ul>
	<li>Необходимо помнить и названия регистров и названия битов, либо постоянно обращаться к мануалу</li>
<li>Легко допустить ошибку в коде. Читатель, наверняка, заметил, что вместо SPI2 был повторно включен USART1 </li>
<li>Для работы некоторых периферийных блоков требуется также включать другую периферию, например, GPIO и DMA для интерфейсов </li>
<li>Полное отсутствие переносимости. При выборе другого контроллера этот код теряет смысл</li>
</ul>

####
При всех недостатках, этот способ остается весьма востребованным, по крайней мере тогда, когда нужно «пощупать» новый контроллер, <s>написав очередной «Hello, World!»</s>!» мигнув светодиодом.  

<h3>Функции инициализации</h3>

####
Давайте попробуем абстрагироваться и спрятать работу с регистрами от пользователя. И в этом нам поможет обыкновенная C-функция:

```cpp
void UART1_Init(){
 RCC->AHBENR  |= RCC_AHBENR_DMA1EN;
 RCC->APB2ENR |= RCC_APB2ENR_IOPAEN
              |  RCC_APB2ENR_USART1EN;
  // Остальная инициализация
}

void SPI2_Init(){
 RCC->AHBENR  |= RCC_AHBENR_DMA1EN;
 RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
 RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
  // Остальная инициализация
}

int main(){
  UART1_Init();
  SPI2_Init();
…
}
```
Размер кода: 72 байта. <a href="https://godbolt.org/z/K5vqa8">Посмотреть</a>

####
<details> 
  <summary>Листинг</summary>
<p>

```s
UART1_Init():
    // AHBENR(Включение DMA1)
  ldr     r2, .L2
  ldr     r3, [r2, #20]
  orr     r3, r3, #1
  str     r3, [r2, #20]
    // APB2ENR(Включение GPIOA, USART1)
  ldr     r3, [r2, #24]
  orr     r3, r3, #16384
  orr     r3, r3, #4
  str     r3, [r2, #24]
  bx      lr
SPI2_Init():
    //Повторно (!) AHBENR(Включение DMA1)
  ldr     r3, .L5
  ldr     r2, [r3, #20]
  orr     r2, r2, #1
  str     r2, [r3, #20]
    //Повторно (!) APB2ENR(Включение GPIOB)
  ldr     r2, [r3, #24]
  orr     r2, r2, #8
  str     r2, [r3, #24]
    //Запись в APB1ENR(Включение SPI2)
  ldr     r2, [r3, #28]
  orr     r2, r2, #16384
  str     r2, [r3, #28]
  bx      lr
main:
   push    {r3, lr}
   bl      UART1_Init()
   bl      SPI2_Init()
```
</p>
</details>

####
<b>Плюсы:</b>
<ul>
	<li>Можно не заглядывать в мануал по каждому поводу</li>
<li>Ошибки локализованы на этапе написания драйвера периферии</li>
<li>Пользовательский код легко воспринимать</li>
</ul>

<b>Минусы:</b>
<ul>
	<li>Количество необходимых инструкций возросло кратно количеству задействованной периферии </li>
<li>Очень много дублирования кода - для каждого номера UART и SPI он будет фактически идентичен</li>
</ul>

####
Хоть мы и избавились от прямой записи в регистры в пользовательском коде, но какой ценой? Требуемый размер памяти и время исполнения для включения увеличились в 2 раза и продолжат расти, при большем количестве задействованной периферии. 

<h3>Функция включения тактирования</h3>

##
Обернем модификацию регистров тактирования в отдельную функцию, предполагая, что это снизит количество необходимой памяти. Вместе с этим, введем параметр-идентификатор для периферии – для уменьшения кода драйверов:

```cpp
void PowerEnable(uint32_t ahb, uint32_t apb2, uint32_t apb1){
    RCC->AHBENR  |= ahb;
    RCC->APB2ENR |= apb2;
    RCC->APB1ENR |= apb1;
}

void UART_Init(int identifier){
    uint32_t ahb = RCC_AHBENR_DMA1EN, apb1 = 0, apb2 = 0;
    if (identifier == 1){
      apb2 |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;
    } 
    else if (identifier == 2){…}
    PowerEnable(ahb, apb2, apb1);
  // Остальная инициализация
}

void SPI_Init(int identifier){
    uint32_t ahb = RCC_AHBENR_DMA1EN, apb1 = 0, apb2 = 0;
    if (identifier == 1){…} 
    else if (identifier == 2){
      apb2 |= RCC_APB2ENR_IOPBEN;
      apb1 = RCC_APB1ENR_SPI2EN;
    }
    PowerEnable(ahb, apb2, apb1);
  // Остальная инициализация
}

int main(){
  UART_Init(1);
  SPI_Init(2);
…
}
```

Размер кода: 92 байта. <a href="https://godbolt.org/z/Gnahz4">Посмотреть</a>

####
<details> 
  <summary>Листинг</summary>
<p>

```s
PowerEnable(unsigned long, unsigned long, unsigned long):
  push    {r4}
  ldr     r3, .L3
  ldr     r4, [r3, #20]
  orrs    r4, r4, r0
  str     r4, [r3, #20]
  ldr     r0, [r3, #24]
  orrs    r0, r0, r1
  str     r0, [r3, #24]
  ldr     r1, [r3, #28]
  orrs    r1, r1, r2
  str     r1, [r3, #28]
  pop     {r4}
  bx      lr
UART_Init(int):
  push    {r3, lr}
  cmp     r0, #1
  mov     r2, #0
  movw    r1, #16388
  it      ne
  movne   r1, r2
  movs    r0, #1
  bl      PowerEnable(unsigned long, unsigned long, unsigned long)
  pop     {r3, pc}
SPI_Init(int):
  push    {r3, lr}
  cmp     r0, #2
  ittee   eq
  moveq   r1, #8
  moveq   r1, #16384
  movne   r1, #0
  movne   r2, r1
  movs    r0, #1
  bl      PowerEnable(unsigned long, unsigned long, unsigned long)
  pop     {r3, pc}
main:
   push    {r3, lr}
   movs    r0, #1
   bl      UART_Init(int)
   movs    r0, #2
   bl      SPI_Init(int)
```
</p>
</details>

####
<b>Плюсы:</b>
<ul>
	<li>Удалось сократить код описания драйверов микроконтроллера</li>
<li>Результирующее количество инструкций сократилось*</li>
<li></li>
</ul>
<b>Минусы:</b>
<ul>
	<li>Увеличилось время выполнения</li>
</ul>

####
*Да, в данном случае размер исполняемого кода возрос, по сравнению с предыдущим вариантом, но это связано с появлением условных операторов, влияние которых можно нивелировать, если задействовать хотя бы по 2 экземпляра каждого вида периферии. 

Т.к. функция включения принимает параметры, то в ассемблере появились операции работы со стеком, что негативно сказывается на производительности.

На этом моменте, думаю, что <s>наши полномочия все</s> стоит переходить к <i>плюсам</i>, потому что основные подходы, используемые в чистом C рассмотрены, за исключением макросов. Но этот способ также далеко не оптимален и сопряжен  с потенциальной вероятностью совершить ошибку в пользовательском коде.  
 
<h3>Свойства-значения и шаблоны</h3>

##
Начиная рассматривать плюсовый подход, сразу пропустим вариант включения тактирования в конструкторе класса, т.к. этот метод фактически не отличается от инициализирующих функций в стиле C. 
Поскольку на этапе компиляции нам известны все значения, которые нужно записать в регистры, то избавимся от операций со стеком. Для этого создадим отдельный класс с шаблонным методом, а классы периферии наделим свойствами (value trait), которые будут хранить значения для соответствующих регистров.

```cpp
struct Power{
template< uint32_t valueAHBENR, uint32_t valueAPB2ENR, uint32_t valueAPB1ENR>
    static void Enable(){
// Если значение = 0, то в результирующем коде операций с регистром не будет
        if constexpr (valueAHBENR)
            RCC->AHBENR |= valueAHBENR;
        if constexpr (valueAPB2ENR)
            RCC->APB2ENR |= valueAPB2ENR;
        if constexpr (valueAPB1ENR)
            RCC->APB1ENR |= valueAPB1ENR;
    };

};

template<auto identifier>
struct UART{
// С помощью identifier на этапе компиляции можно выбрать значения для периферии
  static constexpr auto valueAHBENR = RCC_AHBENR_DMA1EN;
  static constexpr auto valueAPB1ENR = identifier == 1 ? 0U : RCC_APB1ENR_USART2EN;
  static constexpr auto valueAPB2ENR = RCC_APB2ENR_IOPAEN
                                    |  (identifier == 1 ? RCC_APB2ENR_USART1EN : 0U);
    // Остальная реализация
};

template<auto identifier>
struct SPI{
  static constexpr auto valueAHBENR = RCC_AHBENR_DMA1EN;
  static constexpr auto valueAPB1ENR = identifier == 1 ? 0U : RCC_APB1ENR_SPI2EN;
  static constexpr auto valueAPB2ENR = RCC_APB2ENR_IOPBEN
                                    |  (identifier == 1 ? RCC_APB2ENR_SPI1EN : 0U);
    // Остальная реализация
};

int main(){
    // Необязательные псевдонимы для используемой периферии
  using uart = UART<1>;
  using spi = SPI<2>;

  Power::Enable<
                uart::valueAHBENR  | spi::valueAHBENR,
                uart::valueAPB2ENR | spi::valueAPB2ENR,
                uart::valueAPB1ENR | spi::valueAPB1ENR
                >();
…
}
```
Размер кода: 36 байт. <a href="https://godbolt.org/z/a56are">Посмотреть</a>

####
<details> 
  <summary>Листинг</summary>
<p>

```s
    // AHBENR(Включение DMA1)
  ldr     r3, .L3
  ldr     r2, [r3, #20]
  orr     r2, r2, #1
  str     r2, [r3, #20]
    // APB2ENR(Включение GPIOA, GPIOB, USART1)
  ldr     r2, [r3, #24]
  orr     r2, r2, #16384
  orr     r2, r2, #12
  str     r2, [r3, #24]
    // APB1ENR(Включение SPI2)
  ldr     r2, [r3, #28]
  orr     r2, r2, #16384
  str     r2, [r3, #28]
```
</p>
</details>

####
<b>Плюсы:</b>
<ul>
	<li>Размер и время выполнения получились такими же, как и в эталонном варианте с прямой записью в регистры </li>
<li>Довольно просто масштабировать проект – достаточно добавить <s>воды </s> соответствующее свойство-значение периферии</li>
</ul>

<b>Минусы:</b>
<ul>
	<li>Можно совершить ошибку, поставив свойство-значение не в тот параметр</li>
<li> Как и в случае с прямой записью в регистры – страдает переносимость</li>
<li>«Перегруженность» конструкции </li>
</ul>

Несколько поставленных целей мы смогли достигнуть, но удобно ли этим пользоваться? Думаю - нет, ведь для добавления очередного блока периферии необходимо контролировать правильность расстановки свойств классов в параметры шаблона метода.

<h3> Идеальный вариант… почти</h3>

##
Чтобы уменьшить количество пользовательского кода и возможностей для ошибок воспользуемся parameter pack, который уберет обращение к свойствам классов периферии в пользовательском коде. При этом изменится только метод включения тактирования:

```cpp
struct Power{
template<typename... Peripherals>
  static void Enable(){
      // Для всех параметров пакета будет применена операция | 
      // В нашем случае value = uart::valueAHBENR | spi::valueAHBENR и т.д.
    if constexpr (constexpr auto value = (Peripherals::valueAHBENR | ... ); value)
      RCC->AHBENR |= value;
    if constexpr (constexpr auto value = (Peripherals::valueAPB2ENR | ... ); value)
      RCC->APB2ENR |= value;
    if constexpr (constexpr auto value = (Peripherals::valueAPB1ENR | ... ); value)
      RCC->APB1ENR |= value;
  };
};
    …
int main(){
    // Необязательные псевдонимы для используемой периферии
  using uart = UART<1>;
  using spi = SPI<2>;

  Power::Enable<uart, spi>();
…
}
```
Размер кода: 36 байт. <a href="https://godbolt.org/z/9dx98M">Посмотреть</a>

####
<details> 
  <summary>Листинг</summary>
<p>

```s
main:
    // AHBENR(Включение DMA1)
  ldr     r3, .L3
  ldr     r2, [r3, #20]
  orr     r2, r2, #1
  str     r2, [r3, #20]
    // APB2ENR(Включение GPIOA, GPIOB, USART1)
  ldr     r2, [r3, #24]
  orr     r2, r2, #16384
  orr     r2, r2, #12
  str     r2, [r3, #24]
    // APB1ENR(Включение SPI2)
  ldr     r2, [r3, #28]
  orr     r2, r2, #16384
  str     r2, [r3, #28]
```
</p>
</details>

####
Относительно прошлого варианта значительно повысилась простота пользовательского кода, вероятность ошибки стала минимальна, а расход памяти остался на том же уровне.

И, вроде бы, можно на этом остановиться, но…
 
<h2>Расширяем функционал</h2>

##
Обратимся к одной из поставленных целей:
####
<blockquote>Помимо основных возможностей включения и выключения тактирования периферии необходим расширенный функционал </blockquote>

####
Предположим, что стоит задача сделать устройство малопотребляющим, а для этого, естественно, требуется отключать всю периферию, которую контроллер не использует для выхода из энергосберегающего режима.

В контексте условий, озвученных в начале статьи, будем считать, что генератором события пробуждения будет USART1, а SPI2 и соответствующий ему порт GPIOB - необходимо отключать. При этом, общиий ресурс DMA1 должен оставаться включенным.

Воспользовавшись любым вариантом из прошлого раздела, не получится решить эту задачу и эффективно, и оптимально, и, при этом, не используя ручной контроль задействованных блоков. 
Например, возьмем последний способ:

```cpp
int main(){
  using uart = UART<1>;
  using spi = SPI<2>;
…
    // Включаем USART, SPI, DMA, GPIOA, GPIOB
  Power::Enable<uart, spi>();

    // Some code

    // Выключаем SPI и GPIOB вместе (!) с DMA
  Power::Disable<spi>();
    
    // Включаем обратно DMA вместе(!) с USART и GPIOA
  Power::Enable<uart>();
    
    // Sleep();

    // Включаем SPI и GPIOB вместе(!) с DMA
  Power::Enable<spi>();
…
}
```
Размер кода: 100 байт. <a href="https://godbolt.org/z/rbodGE">Посмотреть</a>

####
<details> 
  <summary>Листинг</summary>
<p>

```s
main:
        // AHBENR(Включение DMA1)
        ldr     r3, .L3
        ldr     r2, [r3, #20]
        orr     r2, r2, #1
        str     r2, [r3, #20]
       // APB2ENR(Включение GPIOA, GPIOB, USART1)
        ldr     r2, [r3, #24]
        orr     r2, r2, #16384
        orr     r2, r2, #12
        str     r2, [r3, #24]
       // APB1ENR(Включение SPI2)
        ldr     r2, [r3, #28]
        orr     r2, r2, #16384
        str     r2, [r3, #28]
        // Выключение SPI2
       // AHBENR(Выключение DMA1)
        ldr     r2, [r3, #20]
        bic     r2, r2, #1
        str     r2, [r3, #20]
       // APB2ENR(Выключение GPIOB)
        ldr     r2, [r3, #24]
        bic     r2, r2, #8
        str     r2, [r3, #24]
       // APB1ENR(Выключение SPI2)
        ldr     r2, [r3, #28]
        bic     r2, r2, #16384
        str     r2, [r3, #28]
        // Повторное (!) включение USART1
        // AHBENR(Включение DMA1)
        ldr     r2, [r3, #20]
        orr     r2, r2, #1
        str     r2, [r3, #20]
       // APB2ENR(Включение GPIOA, USART1)
        ldr     r2, [r3, #24]
        orr     r2, r2, #16384
        orr     r2, r2, #4
        str     r2, [r3, #24]
        // Sleep();
        // AHBENR(Включение DMA1)
        ldr     r2, [r3, #20]
        orr     r2, r2, #1
        str     r2, [r3, #20]
       // APB2ENR(Включение GPIOB)
        ldr     r2, [r3, #24]
        orr     r2, r2, #8
        str     r2, [r3, #24]
       // APB1ENR(Включение SPI2)
        ldr     r2, [r3, #28]
        orr     r2, r2, #16384
        str     r2, [r3, #28]
```
</p>
</details>

####
В это же время эталонный код  на регистрах занял 68 байт. <a href="https://godbolt.org/z/PsfqGn">Посмотреть</a>

Очевидно, что для подобных задач, камнем преткновения станут разделяемые ресурсы, такие как DMA. Кроме того, конкретно в этом случае возникнет момент, когда оба интерфейса станут неработоспособными и, фактически, произойдет аварийная ситуация.

Давайте попробуем найти решение…

 
<h2>Структура</h2>

##
Для упрощения понимания и разработки - изобразим общую структуру тактирования, какой мы ее хотим видеть:

<img src="https://habrastorage.org/webt/xh/wi/8g/xhwi8gnq8hjxiajczumohxt_qy4.png" />

Она состоит всего из четырех блоков:

Независимые:
<ul>
	<li> <b>IPower</b> – интерфейс взаимодействия с пользователем, который подготавливает данные для записи в регистры</li>
	<li> <b>Hardware</b>– прямая запись в регистры</li>
</ul>

####
Аппаратно-зависимые:
<ul>
<li> <b> Peripherals</b> – периферия, которая используется в проекте и сообщает интерфейсу, какие устройства надо включить или выключить </li>
	<li> <b>Adapter</b> – передает значения для записи в Hardware, указывая в какие именно регистры их следует записать </li>
</ul> 
<h3>Интерфейс IPower</h3>

##
С учетом всех требований, определим методы, необходимые в интерфейсе:

```cpp
template<typename… Peripherals>
Enable();

template<typename EnableList, typename ExceptList>
EnableExcept();

template<typename EnableList, typename DisableList>
Keep();
```
<b>Enable</b> - включение периферии, указанной в параметре шаблона.

<b>EnableExcept</b> – включение периферии, указанной в параметре EnableList, за исключением той, что указана в ExceptList.

####
<details> 
  <summary>Пояснение</summary>
<p>

Таблица истинности
<table>
  <tr>
    <th>Бит включения</th>
    <th>Бит исключения</th>
     <th>Результат включение</th>
                <th>Результат выключение</th>
  </tr>
  <tr>
    <td align="center">0</td>
    <td align="center">0</td>
    <td align="center">0</td>
                <td align="center">0</td>
  </tr>
  <tr> 
    <td align="center">0</td>
    <td align="center">1</td>
    <td align="center">0</td>
                <td align="center">0</td>
  </tr>
  <tr> 
    <td align="center">1</td>
    <td align="center">0</td>
    <td align="center"><b>1</b></td>
                <td align="center">0</td>
  </tr>
  <tr> 
    <td align="center">1</td>
    <td align="center">1</td>
    <td align="center">0</td>
                <td align="center">0</td>
  </tr>
</table>

Например, вызов :
```cpp
EnableExcept<spi, uart>();
```
должен установить бит SPI2EN и бит IOPBEN. В то время, как общий DMA1EN, а также USART1EN и IOPAEN останутся в исходном состоянии. 

Чтобы получить соответствующую таблицу истинности, необходимо произвести следующие операции:

```cpp
resultEnable = (enable ^ except) & enable
```
</p>
</details>

####
К ним в дополнение также идут комплементарные методы <b>Disable</b>, выполняющие противоположные действия.

<b>Keep</b> – включение периферии из EnableList, выключение периферии из DisableList, при этом, если периферия присутствует в обоих списках, то она сохраняет свое состояние.

####
<details> 
  <summary>Пояснение</summary>
<p>

Таблица истинности
<table>
  <tr>
    <th>Бит включения</th>
    <th>Бит выключения</th>
     <th>Результат включение</th>
            <th>Результат выключение</th>
  </tr>
  <tr>
    <td align="center">0</td>
    <td align="center">0</td>
    <td align="center">0</td>
            <td align="center">0</td>
  </tr>
  <tr> 
    <td align="center">0</td>
    <td align="center">1</td>
    <td align="center">0</td>
    <td align="center">1</td>
  </tr>
  <tr> 
    <td align="center">1</td>
    <td align="center">0</td>
    <td align="center">1</td>
                <td align="center">0</td>
  </tr>
  <tr> 
    <td align="center">1</td>
    <td align="center">1</td>
    <td align="center"><b>0</b></td>
              <td align="center"><b>0</b></td>
  </tr>
</table>

Например, при вызове:
```cpp
Keep<spi, uart>();
```
установятся SPI2EN и IOPBEN, при этом USART1EN и IOPAEN сбросятся, а DMA1EN останется неизменным.

Чтобы получить соответствующую таблицу истинности, необходимо произвести следующие операции:

```cpp
resultEnable = (enable ^ disable) & enable
resultDisable = (enable ^ disable) & disable
```
</p>
</details>

####
Методы включения/выключения уже реализованы довольно неплохо с помощью fold expression, но как быть с остальными?

Если ограничиться использованием 2 видов периферии, как это сделано в пояснении, то никаких сложностей не возникнет. Однако, когда в проекте используется много различных периферийных устройств, появляется проблема – в шаблоне нельзя явно использовать более одного parameter pack, т.к. компилятор не сможет определить где заканчивается один и начинается второй:

```cpp
template<typename… EnableList, typename… ExceptList>
EnableExcept(){…};
  // Невозможно определить где заканчивается EnableList и начинается ExceptList
EnableExcept<spi2, pin3, uart1, pin1, i2c3>();
```
Можно было бы создать отдельный класс-обертку для периферии и передавать его в метод:

```cpp
template<typename… Peripherals>
PowerWrap{
  static constexpr auto valueAHBENR = (Peripherals::valueAHBENR | …);
  static constexpr auto valueAPB1ENR = (Peripherals:: valueAPB1ENR | …);
  static constexpr auto valueAPB2ENR = (Peripherals:: valueAPB2ENR | …);
};

using EnableList = PowerWrap<spi2, uart1>;
using ExceptList = PowerWrap<pin1, i2c1>;

EnableExcept<EnableList, ExceptList>();
```
Но и в этом случае, интерфейс станет жестко завязан на количестве регистров, следовательно, для каждого типа контроллера станет необходимо писать свой отдельный класс, с множеством однотипных операций и без возможности разделения на абстрактные слои.

Поскольку вся используемая периферия и регистры тактирования известны на этапе компиляции, то поставленную задачу возможно решить с использованием метапрограммирования.
 
<h4>Метапрограммирование</h4>

##
Из-за того, что в основе метапрограммирования лежит работа не с обычными типами, а с их списками, то определим две сущности, которые будут оперировать типовыми и нетиповыми параметрами:

```cpp
template<typename... Types>
struct Typelist{};

template<auto... Values>
struct Valuelist{};
…
using listT = Typelist<char, int> ;// Список из последовательности типов char и int
…
using listV = Valuelist<8,9,5,11> ;// Список из 4 нетиповых параметров
```
Прежде чем делать что-либо полезное с этими списками, нам потребуется реализовать некоторые базовые операции, на основе которых станет возможно совершать более сложные действия.

1. Извлечение первого элемента из списка
####
<details> 
  <summary>front</summary>
<p>

```cpp
  // Прототип функции
template<typename List>
struct front;

  // Специализация для списка типов
  // Разделение списка в пакете параметров на заглавный и оставшиеся
template<typename Head, typename... Tail>
struct front<Typelist<Head, Tail...>>{ 
    // Возвращение заглавного типа
  using type = Head; 
};

 // Специализация для списка нетиповых параметров
template<auto Head, auto... Tail>
struct front<Valuelist<Head, Tail...>> {
  // Возвращение заглавного значения
  static constexpr auto value = Head;
};

  // Псевдонимы для простоты использования
template<typename List>
using front_t = typename front<List>::type;

template<typename List>
static constexpr auto front_v = front<List>::value;

  // Примеры
using listT = Typelist<char, bool, int>;
using type = front_t<listT>; // type = char

using listV = Valuelist<9,8,7>;
constexpr auto value = front_v<listV>; //value = 9
```
</p>
</details>

####
2. Удаление первого элемента из списка
####
<details> 
  <summary>pop_front</summary>
<p>

```cpp
template<typename List>
struct pop_front;

  // Специализация для списка типов
  // Разделение списка в пакете параметров на заглавный и оставшиеся
template<typename Head, typename... Tail>
struct pop_front<Typelist<Head, Tail...>> {
  // Возвращение списка, содержащего оставшиеся типы
  using type = Typelist<Tail...>;
};

template<auto Head, auto... Tail>
struct pop_front<Valuelist<Head, Tail...>> {
  using type = Valuelist<Tail...>;
};

template<typename List>
using pop_front_t = typename pop_front<List>::type;

 // Примеры
using listT = Typelist<char, bool, int>;
using typeT = pop_front_t<listT>; // type = Typelist<bool, int>

using listV = Valuelist<9,8,7>;
using typeV = pop_front_t<listV>; // type = Valuelist<8,7>
```
</p>
</details>

####
3. Добавление элемента в начало списка
####
<details> 
  <summary>push_front</summary>
<p>

```cpp
template<typename List, typename NewElement>
struct push_front;

template<typename... List, typename NewElement>
struct push_front<Typelist<List...>, NewElement> {
  using type = Typelist<NewElement, List...>;
};

template<typename List, typename NewElement>
using push_front_t = typename push_front<List, NewElement>::type;

// Пример
using listT = Typelist<char, bool, int>;
using typeT = push_front_t<listT, long >; // type = Typelist<long, char, bool, int>
```
</p>
</details>

####
4. Добавление нетипового параметра в конец списка
####
<details> 
  <summary>push_back</summary>
<p>

```cpp
template<typename List, auto NewElement>
struct push_back;

template<auto... List, auto NewElement>
struct push_back<Valuelist<List...>, NewElement>{
  using type = Valuelist<List..., NewElement>;
};

template<typename List, auto NewElement>
using push_back_t = typename push_back<List, NewElement>::type;

using listV = Valuelist<9,8,7>;
using typeV = push_back_t<listV, 6>; // typeV = Valuelist<9,8,7,6>
```
</p>
</details>

####
5. Проверка списка на пустоту
####
<details> 
  <summary>is_empty</summary>
<p>

```cpp
template<typename List>
struct is_empty{
    static constexpr auto value = false;
};

 // Специализация для базового случая, когда список пуст
template<>
struct is_empty<Typelist<>>{
    static constexpr auto value = true;
};

template<typename List>
static constexpr auto is_empty_v = is_empty<List>::value;

 // Пример
using listT = Typelist<char, bool, int>;
constexpr auto value = is_empty_v<listT>; // value = false
```
</p>
</details>

####
6. Нахождение количества элементов в списке
####
<details> 
  <summary>size_of_list</summary>
<p>

```cpp
  // Функция рекурсивно извлекает по одному элементу из списка,
  // инкрементируя счетчик count, пока не дойдет до одного из 2 базовых случаев
template<typename List, std::size_t count = 0>
struct size_of_list : public size_of_list<pop_front_t<List>, count + 1>{};

  // Базовый случай для пустого списка типов
template<std::size_t count>
struct size_of_list<Typelist<>, count>{
  static constexpr std::size_t value = count;
};

  // Базовый случай для пустого списка нетиповых параметров 
template<std::size_t count>
struct size_of_list<Valuelist<>, count>{
  static constexpr std::size_t value = count;
};

template<typename List>
static constexpr std::size_t size_of_list_v = size_of_list<List>::value;

  // Пример
using listT = Typelist<char, bool, int>;
constexpr auto value = size_of_list_v <listT>; // value = 3
```
</p>
</details>

####
Теперь, когда определены все базовые действия, можно переходить к написанию метафункций для битовых операций: <b>or</b>, <b>and</b>, <b>xor</b>, необходимых для методов интерфейса.
Поскольку эти битовые преобразования однотипны, то попытаемся сделать реализацию максимально обобщенно, чтобы избежать дублирования кода. 

Функция, выполняющая абстрактную операцию над списком
####
<details> 
  <summary>lists_operation</summary>
<p>

```cpp
template<template<typename first, typename second> class operation,
         typename Lists, bool isEnd = size_of_list_v<Lists> == 1>
class lists_operation{

  using first = front_t<Lists>; // (3)
  using second = front_t<pop_front_t<Lists>>; // (4)
  using next = pop_front_t<pop_front_t<Lists>>; // (5)
  using result = operation<first, second>; // (6)

public:

  using type = typename 
      lists_operation<operation, push_front_t<next, result>>::type; // (7)

};

template<template<typename first, typename second> class operation, typename List>
class lists_operation<operation, List, true>{ // (1)
public:
  using type = front_t<List>; // (2)
};
```
<b>Lists</b> – список, состоящий из типов или списков, над которым необходимо провести некоторое действие.
<b>operation</b> – функциональный адаптер, который принимает 2 первых элемента Lists и возвращает результирующий тип после операции.
<b>isEnd</b> – граничное условие метафункции, которое проверяет количество типов в Lists.

В базовом случае (1) Lists состоит из 1 элемента, поэтому результатом работы функции станет его извлечение(2).
Для остальных случаев – определяют первый (3) и второй (4) элементы из Lists, к которым применяется <i>операция</i> (6). Для получения результирующего типа (7) происходит рекурсивный вызов метафункции с новым списком типов, на первом месте которого стоит (6), за которым следуют оставшиеся типы (5) исходного Lists. Окончанием рекурсии становиться вызов специализации (1).  
</p>
</details>

####
Далее, реализуем операцию для предыдущей метафункции, которая почленно будет производить абстрактные действия над нетиповыми параметрами из двух списков:
####
<details> 
  <summary>valuelists_operation</summary>
<p>

```cpp
template<template <auto value1, auto value2> typename operation, 
        typename List1, typename List2, typename Result = Valuelist<>>
struct operation_2_termwise_valuelists{
  constexpr static auto newValue = 
      operation<front_v<List1>, front_v<List2>>::value; // (2)
  
  using nextList1 = pop_front_t<List1>;
  using nextList2 = pop_front_t<List2>;
    
  using result = push_back_value_t<Result, newValue>; // (3)
  using type = typename 
      operation_2_termwise_valuelists <operation, nextList1, nextList2, result>::type; // (4)
};

template<template <auto value1, auto value2> typename operation, typename Result>
struct operation_2_termwise_valuelists <operation, Valuelist<>, Valuelist<>, Result>{ // (1)
  using type = Result;
};
```
<b>List1</b> и <b>List2</b> – списки нетиповых параметров, над которыми необходимо произвести действие.
<b>operation</b> – операция, производимая над нетиповыми параметрами.
<b>Result</b> – тип, используемый для накопления промежуточных результатов. 

Базовый случай (1), когда оба списка пусты, возвращает Result.
Для остальных случаев происходит вычисление значения операции (2) и занесение его в результирующий список Result (3). Далее рекурсивно вызывается метафункция (4) до того момента, пока оба списка не станут пустыми.   
</p>
</details>

####
Функции битовых операций:
####
<details> 
  <summary>bitwise_operation</summary>
<p>

```cpp
template<auto value1, auto value2>
struct and_operation{ static constexpr auto value = value1 & value2;};

template<auto value1, auto value2>
struct or_operation{ static constexpr auto value = value1 | value2;};

template<auto value1, auto value2>
struct xor_operation{ static constexpr auto value = value1 ^ value2;};
```
</p>
</details>

####
Осталось создать псевдонимы для более простого использования:
####
<details> 
  <summary>псевдонимы</summary>
<p>

```cpp
  // Псевдонимы для битовых почленных операций над 2 списками
template<typename List1, typename List2>
using operation_and_termwise_t = typename 
          operation_2_termwise_valuelists<and_operation, List1, List2>::type;

template<typename List1, typename List2>
using operation_or_termwise_t = typename 
          operation_2_termwise_valuelists<or_operation, List1, List2>::type;

template<typename List1, typename List2>
using operation_xor_termwise_t = typename 
          operation_2_termwise_valuelists<xor_operation, List1, List2>::type;

  // Псевдонимы почленных битовых операций для произвольного количества списков
template<typename... Lists>
using lists_termwise_and_t = typename 
          lists_operation<operation_and_termwise_t, Typelist<Lists...>>::type;

template<typename... Lists>
using lists_termwise_or_t= typename 
          lists_operation<operation_or_termwise_t, Typelist<Lists...>>::type;

template<typename... Lists>
using lists_termwise_xor_t = typename 
          lists_operation<operation_xor_termwise_t, Typelist<Lists...>>::type;
```
<a href="https://godbolt.org/z/9nYjqT">Пример использования</a> (Обратите внимание на вывод ошибок).
</p>
</details>

####
 
<h4>Возвращаясь к имплементации интерфейса</h4>

##
Поскольку на этапе компиляции известен и контроллер, и используемая периферия, то логичный выбор для реализации интерфейса - статический полиморфизм с идиомой <abbr title="Curiously recurring template pattern"><a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a></abbr>. В качестве шаблонного параметра, интерфейс принимает класс-адаптер конкретного контроллера, который, в свою очередь, является наследником этого интерфейса.

```cpp
template<typename adapter>  
struct IPower{

  template<typename... Peripherals>
  static void Enable(){
     
      // Раскрытие пакета параметров периферии, содержащей свойство ‘power’
      // и применение побитового или к значениям
    using tEnableList = lists_termwise_or_t<typename Peripherals::power...>;

      // Псевдоним Valuelist<…>, содержащий только 0, 
      // количество которых равно количеству регистров
    using tDisableList = typename adapter::template fromValues<>::power;
   
    adapter:: template _Set<EnableList, DisableList>();
  }

  template<typename EnableList, typename ExceptList>
  static void EnableExcept(){
    using tXORedList = lists_termwise_xor_t <
        typename EnableList::power, typename ExceptList::power>;
    using tEnableList = lists_termwise_and_t <
        typename EnableList::power, tXORedList>;
    using tDisableList = typename adapter::template fromValues<>::power;
      // Передача списков включения/отключения адаптеру 
    adapter:: template _Set<EnableList, DisableList>();
  }

  template<typename EnableList, typename DisableList>
    static void Keep(){
    using tXORedList = lists_termwise_xor_t <
        typename EnableList::power, typename DisableList::power>;
    using tEnableList = lists_termwise_and_t <
        typename EnableList::power, tXORedList>;
    using tDisableList = lists_termwise_and_t <
        typename DisableList::power, tXORedList>;
    adapter:: template _Set<EnableList, DisableList>();
  }

  template<typename... PeripheralsList>
  struct fromPeripherals{
    using power = lists_termwise_or_t<typename PeripheralsList::power...>;
  };

};
```

Также, интерфейс содержит встроенный класс <b>fromPeripherals</b>, позволяющий объединять периферию в один список, который, затем, можно использовать в методах:

```cpp
  using listPower = Power::fromPeripherals<spi, uart>;

  Power::Enable<listPower>();
```

Методы <b>Disable </b> реализуются аналогично.
 
<h3>Адаптер контроллера</h3>

####
В классе адаптера необходимо задать адреса регистров тактирования и определить последовательность, в которой будет производиться запись в них, а затем передать управление непосредственно классу, который установит или сбросит биты указанных регистров.

```cpp
struct Power: public IPower<Power>{

  static constexpr uint32_t 
    _addressAHBENR  = 0x40021014,
    _addressAPB2ENR = 0x40021018,
    _addressAPB1ENR = 0x4002101C;
  
  using AddressesList = Valuelist<
      _addressAHBENR, _addressAPB1ENR, _addressAPB2ENR>;

  template<typename EnableList, typename DisableList>
  static void _Set(){
    // Вызов метода класса, осуществляющий запись в регистры
    HPower:: template ModifyRegisters<EnableList, DisableList, AddressesList>();
  }
    
  template<uint32_t valueAHBENR = 0, uint32_t valueAPB1ENR = 0, uint32_t valueAPB2ENR = 0>
  struct fromValues{
    using power = Valuelist<valueAHBENR, valueAPB1ENR, valueAPB2ENR>;
  };

};
```
<h3>Периферия</h3>

Наделяем периферию свойством <i>power</i>, используя структуру <i>fromValues</i> адаптера:

```cpp
template<int identifier>
struct SPI{
  // С помощью identifier можно выбирать необходимые биты на этапе компиляции
  using power = Power::fromValues<
      RCC_AHBENR_DMA1EN, // Значения для соответствующих регистров,
      RCC_APB1ENR_SPI2EN, // последовательность которых определена в адаптере
      RCC_APB2ENR_IOPBEN>::power;
};

template<int identifier>
struct UART{
  using power = Power::fromValues<
      RCC_AHBENR_DMA1EN,
      0U, 
      RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN>::power;
};
```

<h3>Запись в регистры</h3>

##
Класс состоит из рекурсивного шаблонного метода, задача которого сводится к записи значений в регистры контроллера, переданные адаптером.
В качестве параметров, метод принимает 3 списка нетиповых параметров <i>Valuelist<…></i>:

<ul>
	<li><b>SetList </b> и <b>ResetList </b> – списки из последовательностей значений битов, которые необходимо установить/сбросить в регистре</li>
	<li><b>AddressesList </b>– список адресов регистров, в которые будет производится запись значений из предыдущих параметров</li>
</ul>

```cpp
struct HPower{

  template<typename SetList, typename ResetList, typename AddressesList>
    static void ModifyRegisters(){
    if constexpr (!is_empty_v<SetList> && !is_empty_v<ResetList> && 
                  !is_empty_v<AddressesList>){

        // Получаем первые значения списков
      constexpr auto valueSet = front_v<SetList>;
      constexpr auto valueReset = front_v<ResetList>;

      if constexpr(valueSet || valueReset){

        constexpr auto address = front_v<AddressesList>;
        using pRegister_t = volatile std::remove_const_t<decltype(address)>* const;
        auto& reg = *reinterpret_cast<pRegister_t>(address);

        // (!)Единственная строчка кода, которая может попасть в ассемблерный листинг
        reg = (reg &(~valueReset)) | valueSet;
      }

        // Убираем первые значения из всех списков            
      using tRestSet = pop_front_t<SetList>;
      using tRestReset = pop_front_t<ResetList>;
      using tRestAddress = pop_front_t<AddressesList>;
      
        // Вызывается до тех пор, пока списки не станут пустыми
      ModifyRegisters<tRestSet, tRestReset, tRestAddress>();
    }
  };

};
```
В классе присутствует единственная строчка кода, которая попадет в ассемблерный листинг.
 
Теперь, когда все блоки структуры готовы, перейдём к тестированию.
 
<h2>Тестируем код</h2>

##
Вспомним условия последней задачи:

<ul>
	<li> Включение SPI2 и USART1 </li>
          <li> Выключение SPI2 перед входом в режим энергосбережения</li>
<li> Включение SPI2 после выхода из режима энергосбережения</li>
</ul>

```cpp
// Необязательные псевдонимы для периферии
using spi = SPI<2>;
using uart = UART<1>;

// Задаем списки управления тактированием (для удобства)
using listPowerInit = Power::fromPeripherals<spi, uart>;
using listPowerDown = Power::fromPeripherals<spi>;
using listPowerWake = Power::fromPeripherals<uart>;

int main() {

   // Включение SPI2, UASRT1, DMA1, GPIOA, GPIOB
    Power::Enable<listPowerInit>();

    // Some code
    
    // Выключение только SPI2 и GPIOB
    Power::DisableExcept<listPowerDown, listPowerWake>();

    //Sleep();

    // Включение только SPI2 и GPIOB
    Power::EnableExcept<listPowerDown, listPowerWake>();
…
}

```
Размер кода: 68 байт*, как и в случае с прямой записью в регистры.

####
<details> 
  <summary>Листинг</summary>
<p>

```s
main:
  // AHBENR(Включение DMA1)
  ldr     r3, .L3
  ldr     r2, [r3, #20]
  orr     r2, r2, #1
  str     r2, [r3, #20]
  // APB1ENR(Включение SPI2
  ldr     r2, [r3, #28]
  orr     r2, r2, #16384
  str     r2, [r3, #28]
  // APB2ENR(Включение GPIOA, GPIOB, USART1)
  ldr     r2, [r3, #24]
  orr     r2, r2, #16384
  orr     r2, r2, #12
  str     r2, [r3, #24]
  // APB1ENR(Выключение SPI2)
  ldr     r2, [r3, #28]
  bic     r2, r2, #16384
  str     r2, [r3, #28]
  // APB2ENR(Выключение GPIOB)
  ldr     r2, [r3, #24]
  bic     r2, r2, #8
  str     r2, [r3, #24]
  // APB1ENR(Включение SPI2
  ldr     r2, [r3, #28]
  orr     r2, r2, #16384
  str     r2, [r3, #28]
  // APB2ENR(Выключение GPIOB)
  ldr     r2, [r3, #24]
  orr     r2, r2, #8
  str     r2, [r3, #24]
```
</p>
</details>

####
*При использовании <b>GCC 9.2.1</b> получается на 8 байт больше, чем в версии <b>GCC 10.1.1</b>. Как видно из <a href="https://godbolt.org/z/q54eeE">листинга</a> - добавляются несколько ненужных инструкций, например, перед чтением по адресу (<b>ldr</b>) есть инструкция добавления (<b>adds</b>), хотя эти инструкции можно заменить на чтение со смещением. Новая версия оптимизирует эти операции. При этом <a href="https://godbolt.org/z/Eae5z7">clang</a> генерирует одинаковые листинги.

 
<h2>Итоги</h2>

##
Поставленные в начале статьи цели достигнуты – скорость выполнения и эффективность сохранились на уровне прямой записи в регистр, вероятность ошибки в пользовательском коде сведена к минимуму.

Возможно, объем исходного кода и сложность разработки покажутся избыточными, однако, благодаря такому количеству абстракций, переход к новому контроллеру займет минимум усилий: 30 строчек понятного кода адаптера + по 5 строк на периферийный блок.
####