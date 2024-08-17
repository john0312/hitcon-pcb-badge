## 1. 創建一個 Udev 規則：

    創建或編輯一個 Udev 規則文件，例如 /etc/udev/rules.d/99-hid.rules。
    添加類似以下內容的規則：

bash

SUBSYSTEM=="hidraw", KERNEL=="hidraw*", MODE="0666"

2. 重新加載 Udev 規則：

    保存文件後，重新加載 Udev 規則：

bash

sudo udevadm control --reload-rules
sudo udevadm trigger
