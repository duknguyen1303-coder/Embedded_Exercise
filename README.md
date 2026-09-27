# Lập trình hệ thống nhúng – Interrupt Handling

Repository này chứa các bài thực hành về **xử lý ngắt (Interrupt Handling)** trong môn **Lập trình hệ thống nhúng**.

Các bài tập được thực hiện trên **STM32F411 Discovery** bằng **STM32CubeIDE**, tập trung vào việc cấu hình và sử dụng các bộ điều khiển ngắt ở mức thanh ghi thông qua CMSIS.
---

## 1. Thông tin Board

- **Board:** STM32F411 Discovery
- **Vi điều khiển:** STM32F411
- **CPU:** ARM Cortex-M4
- **IDE:** STM32CubeIDE
- **Debugger/Programmer:** ST-LINK
- **Phương pháp lập trình:** CMSIS / truy cập thanh ghi trực tiếp

Các ngoại vi và thành phần chính được sử dụng trong các bài:

- GPIO
- NVIC
- EXTI
- SysTick
- SYSCFG

---

## 2. Nội dung các bài tập

### Exercise 01 – NVIC
**Thư mục:** `L39_Interrupt_Ex01_NVIC`
Bài tập làm quen với **Nested Vectored Interrupt Controller (NVIC)**.
Nội dung chính:
- Xác định vị trí của các interrupt trong NVIC.
- Tính toán vị trí bit trong thanh ghi `NVIC->ISER`.
- Enable interrupt bằng cách truy cập trực tiếp thanh ghi.
- Làm quen với thanh ghi thiết lập mức ưu tiên của interrupt.
Các interrupt được sử dụng:
- `EXTI0_IRQn`
- `TIM2_IRQn`
- `USART1_IRQn`
---

### Exercise 02 – EXTI Button
**Thư mục:** `L39_Interrupt_Ex02_EXTI_Button`
Bài tập sử dụng **External Interrupt (EXTI)** để phát hiện thao tác nhấn nút.
Nội dung chính:
- Cấu hình GPIO input.
- Cấu hình EXTI.
- Kết nối chân GPIO với EXTI.
- Enable `EXTI0` trong NVIC.
- Đếm số lần nhấn nút bằng interrupt.
- Xuất giá trị bộ đếm ra GPIO.
- Xóa cờ pending của EXTI.
Nút nhấn trên board được sử dụng tại **PA0**.
---

### Exercise 03 – SysTick
Bài tập này gồm hai chương trình riêng biệt nhằm so sánh cách sử dụng **SysTick bằng polling và interrupt**.
#### 03A – SysTick Polling
**Thư mục:** `L39_Interrupt_Ex03_SysTick_Polling`
SysTick được sử dụng theo phương pháp polling.
Nội dung chính:
- Cấu hình SysTick.
- Xây dựng hàm `delay_ms()`.
- Kiểm tra `COUNTFLAG` để xác định thời gian trễ.
- Điều khiển LED bằng khoảng thời gian tạo bởi SysTick.

#### 03B – SysTick Interrupt
**Thư mục:** `L39_Interrupt_Ex03_SysTick_Interrupt`
SysTick được sử dụng ở chế độ interrupt.
Nội dung chính:
- Cấu hình SysTick tạo interrupt mỗi 1 ms.
- Sử dụng `SysTick_Handler()`.
- Tạo bộ đếm thời gian theo đơn vị mili giây.
- Điều khiển LED thông qua SysTick interrupt.
- So sánh cách hoạt động với phương pháp polling.
- 
### So sánh
**Polling:**
CPU phải liên tục kiểm tra cờ `COUNTFLAG` trong quá trình delay, vì vậy CPU phải chờ trong khoảng thời gian này.
**Interrupt:**
SysTick tự động tạo interrupt theo chu kỳ. CPU không cần liên tục kiểm tra cờ trong `main()`, do đó có thể thực hiện các công việc khác.
---

### Exercise 04 – Interrupt Priority & Nesting
**Thư mục:** `L39_Interrupt_Ex04_EXTI_Priority`
Bài tập tìm hiểu về **mức ưu tiên interrupt và interrupt nesting**.
Hai external interrupt được sử dụng:
- **PA0 → EXTI0 → LED đỏ (PD14)**
- **PA1 → EXTI1 → LED xanh (PD12)**
Nội dung chính:
- Cấu hình hai external interrupt.
- Thiết lập mức ưu tiên cho từng interrupt.
- Quan sát cơ chế ưu tiên interrupt.
- Tìm hiểu interrupt preemption và nesting.
- Thay đổi mức ưu tiên giữa `EXTI0` và `EXTI1`.
- Quan sát sự khác nhau trong quá trình xử lý hai interrupt.
Trong chương trình, `EXTI0_IRQHandler()` thực hiện delay 3 giây để tạo điều kiện quan sát việc một interrupt có mức ưu tiên cao hơn có thể được xử lý trong khi interrupt có mức ưu tiên thấp hơn đang chạy.
---

## 3. Cấu trúc Repository

```text
Embedded_Exercise/
│
├── L39_Interrupt_Ex01_NVIC/
│
├── L39_Interrupt_Ex02_EXTI_Button/
│
├── L39_Interrupt_Ex03_SysTick_Interrupt/
│
├── L39_Interrupt_Ex03_SysTick_Polling/
│
├── L39_Interrupt_Ex04_EXTI_Priority/
│
├── .gitignore
└── README.md
