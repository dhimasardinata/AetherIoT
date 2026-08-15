const translations = {
  en: {
    heroTag: "C++20 Zero-Heap Industrial IoT Framework",
    heroTitle: "Deterministic, High-Performance Embedded IoT",
    heroDesc: "AetherIoT is an ultra-reliable, zero-heap C++20 bare-metal framework designed for industrial telemetry, smart agriculture, hydroponics dosing, weather stations, and high-density multi-actuator systems across all Espressif silicon.",
    quickstartTitle: "5-Line Declarative Quickstart",
    quickstartDesc: "Get up and running in seconds with automatic Wi-Fi failover, real-time web telemetry, and safety range guards.",
    siliconTitle: "Supported Espressif Silicon Matrix",
    actuatorsTitle: "Actuators & Multi-Servo Matrix",
    actuatorsDesc: "High-density multi-servo controller with relay-like declarative syntax (open, close, toggle), 16-channel relays, PCA9685 12-bit PWM, and 74HC595 shift registers.",
    multiplexerTitle: "High-Density Sensor Multiplexing",
    multiplexerDesc: "TCA9548A 8-channel I2C MUX, 1-Wire DS18B20 multi-drop array (16 probes on 1 pin), and CD74HC4067 16-channel analog MUX.",
    securityTitle: "Decoupled Encryption & Anti-Replay",
    securityDesc: "Independent symmetric ciphers (AES-256-CBC, AES-128, ChaCha20, HMAC) paired with an orthogonal anti-replay validation layer.",
    cliTitle: "Custom Extensible CLI Engine",
    cliDesc: "Easily register custom commands with fluent chaining, typed arguments (int, float, bool), rich responses, and auto-generated help menus.",
    lcdTitle: "Custom Dynamic Multi-Page LCD",
    lcdDesc: "Fully user-defined LCD multi-page flipper with dynamic telemetry callbacks, custom labels, and direct cursor printing.",
    csvTitle: "Configurable CSV Profiles & SD Logging",
    csvDesc: "Pre-configured industry CSV presets (Environmental, Energy, Water, Agri, Vibration) with zero-data-loss SD logging and custom formatters.",
    dashboardTitle: "Customizable Embedded Web Dashboard",
    dashboardDesc: "Single-page responsive web dashboard served from Flash RODATA with dynamic telemetry cards, interactive relays, and CLI console.",
    partitionsTitle: "Board Partition Presets (4MB - 32MB)",
    partitionsDesc: "Pre-configured flash memory layouts for dual OTA, edge AI models, crash dumps, and large NVS storage across all board configurations.",
    mqttTitle: "Customizable MQTT Broker & Topic Schema",
    mqttDesc: "Fluent MQTT configuration supporting custom brokers, auth credentials, custom pub/sub topic trees, QoS 0-2, retain, and JSON serializers.",
    modbusTitle: "Modbus RTU / RS-485 Master Engine",
    modbusDesc: "Decoupled Modbus query builder with standard function codes (FC01-FC16), deterministic CRC-16, and response dispatchers.",
    buttonTitle: "Debounced Hardware Buttons & Long-Press",
    buttonDesc: "Fluent digital inputs with active low/high polarity, internal pullups, and chained press, release, and long-press event handlers.",
    webhookTitle: "HTTP REST Webhooks & Push Alerts",
    webhookDesc: "Direct HTTP POST and embedded webhooks for instantaneous Telegram Bot, Discord, and cloud API notifications.",
    navQuickstart: "Quickstart",
    navSilicon: "Hardware Targets",
    navActuators: "Actuators & Servos",
    navMultiplexer: "Sensor Multiplexer",
    navSecurity: "Security & Anti-Replay",
    navCli: "Custom CLI Engine",
    navLcd: "Dynamic LCD Display",
    navCsv: "CSV Logging Profiles",
    navDashboard: "Web Dashboard",
    navPartitions: "Flash Partitions",
    navMqtt: "MQTT Configurator",
    navModbus: "Modbus RTU Engine",
    navButtons: "Hardware Buttons",
    navWebhooks: "REST & Webhooks",
    navExamples: "Production Examples",
    navInstall: "Installation & Registry",
    copyCode: "Copy",
    copiedCode: "Copied!"
  },
  id: {
    heroTag: "Framework IoT Industri C++20 Bebas Alokasi Dinamis (Zero-Heap)",
    heroTitle: "IoT Tertanam Berkinerja Tinggi & Deterministik",
    heroDesc: "AetherIoT adalah framework bare-metal C++20 modern, zero-heap, dan sangat andal untuk telemetri industri, pertanian pintar, hidroponik presisi, stasiun cuaca, dan aktuator multi-servo pada seluruh silikon Espressif.",
    quickstartTitle: "Panduan Cepat 5 Baris Kode",
    quickstartDesc: "Jalankan node IoT dalam hitungan detik dengan failover Wi-Fi otomatis, telemetri web real-time, dan pengaman batas aman.",
    siliconTitle: "Matriks Target Silikon Espressif",
    actuatorsTitle: "Aktuator & Matriks Multi-Servo",
    actuatorsDesc: "Kontrol multi-servo kepadatan tinggi dengan sintaks mirip relai (open, close, toggle), relai 16-channel, PWM PCA9685 12-bit, dan shift register 74HC595.",
    multiplexerTitle: "Multiplexing Sensor Kepadatan Tinggi",
    multiplexerDesc: "TCA9548A 8-channel I2C MUX, array 1-Wire DS18B20 (16 probe pada 1 pin GPIO), dan MUX analog 16-channel CD74HC4067.",
    securityTitle: "Enkripsi Terdekopel & Anti-Replay",
    securityDesc: "Cipher simetris independen (AES-256-CBC, AES-128, ChaCha20, HMAC) yang dipadukan dengan lapisan validasi anti-replay ortogonal.",
    cliTitle: "Engine CLI Kustom yang Fleksibel",
    cliDesc: "Daftarkan perintah CLI kustom dengan mudah menggunakan metode berantai (fluent API), parsing argumen bertipe data (int, float, bool), dan menu bantuan otomatis.",
    lcdTitle: "Layar LCD Multi-Halaman Dinamis Kustom",
    lcdDesc: "Halaman LCD yang 100% dapat ditentukan oleh pengguna dengan callback nilai sensor real-time, label kustom, dan cetak kursor langsung.",
    csvTitle: "Profil Logging CSV & Presets SD Card",
    csvDesc: "Preset CSV industri siap pakai (Lingkungan, Energi, Kualitas Air, Pertanian, Getaran) dengan pencatatan MicroSD anti hilang data dan custom formatter.",
    dashboardTitle: "Web Dashboard Tertanam yang Dapat Dikustomisasi",
    dashboardDesc: "Aplikasi web dashboard responsif dari Flash ROM dengan kartu telemetri dinamis, kontrol relai interaktif, dan konsol terminal CLI.",
    partitionsTitle: "Preset Tabel Partisi Flash Board (4MB - 32MB)",
    partitionsDesc: "Tata letak memori flash siap pakai untuk Dual OTA, model Edge AI, coredump analisis crash, dan penyimpanan NVS besar untuk berbagai varian board.",
    mqttTitle: "Konfigurasi MQTT Broker & Schema Topik Kustom",
    mqttDesc: "Konfigurasi MQTT berantai yang mendukung broker kustom, otentikasi, hierarki topik publish/subscribe bebas, QoS 0-2, retain, dan serializer JSON.",
    modbusTitle: "Master Engine Modbus RTU / RS-485",
    modbusDesc: "Pembangun kueri Modbus terdekopel dengan kode fungsi standar (FC01-FC16), CRC-16 deterministik, dan dispatcher callback respons.",
    buttonTitle: "Tombol Input Hardware & Deteksi Tekan Lama",
    buttonDesc: "Penanganan tombol digital berantai dengan anti-pantul (debounce), polaritas active low/high, dan event handler tekan, lepas, dan long-press.",
    webhookTitle: "Webhook HTTP REST & Notifikasi Instan",
    webhookDesc: "Klien HTTP POST mandiri dan webhook terintegrasi untuk notifikasi instan langsung ke Bot Telegram, Discord, dan REST API cloud.",
    navQuickstart: "Panduan Cepat",
    navSilicon: "Target Hardware",
    navActuators: "Aktuator & Servo",
    navMultiplexer: "Multiplexer Sensor",
    navSecurity: "Keamanan & Anti-Replay",
    navCli: "Engine Custom CLI",
    navLcd: "Layar LCD Dinamis",
    navCsv: "Profil Logging CSV",
    navDashboard: "Web Dashboard",
    navPartitions: "Partisi Flash",
    navMqtt: "Konfigurator MQTT",
    navModbus: "Engine Modbus RTU",
    navButtons: "Tombol & Interupsi",
    navWebhooks: "REST & Webhook",
    navExamples: "Contoh Skenario",
    navInstall: "Panduan Instalasi",
    copyCode: "Salin",
    copiedCode: "Tersalin!"
  }
};

let currentLang = 'en';

function setLanguage(lang) {
  currentLang = lang;
  document.querySelectorAll('.lang-btn').forEach(btn => {
    btn.classList.toggle('active', btn.dataset.lang === lang);
  });

  document.querySelectorAll('[data-i18n]').forEach(el => {
    const key = el.dataset.i18n;
    if (translations[lang] && translations[lang][key]) {
      el.textContent = translations[lang][key];
    }
  });

  document.documentElement.lang = lang;
}

function copyToClipboard(button) {
  const container = button.closest('.code-container');
  const code = container.querySelector('code').innerText;
  navigator.clipboard.writeText(code).then(() => {
    button.textContent = translations[currentLang].copiedCode;
    setTimeout(() => {
      button.textContent = translations[currentLang].copyCode;
    }, 2000);
  });
}

// Search Filter
document.addEventListener('DOMContentLoaded', () => {
  const searchInput = document.getElementById('search-input');
  if (searchInput) {
    searchInput.addEventListener('input', (e) => {
      const query = e.target.value.toLowerCase();
      document.querySelectorAll('section').forEach(sec => {
        const text = sec.innerText.toLowerCase();
        sec.style.display = text.includes(query) ? '' : 'none';
      });
    });
  }
});
