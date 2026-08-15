# Official IoT Platform & Library Registry Submission Guide

This document outlines the exact procedural steps to publish **AetherIoT** to official IoT registries:

---

## 1. PlatformIO Registry Submission

PlatformIO uses the metadata defined in [`library.json`](../library.json).

### Prerequisites:
1. Ensure PlatformIO Core (CLI) is installed:
   ```bash
   pip install platformio
   ```
2. Log in to your PlatformIO account:
   ```bash
   pio account login
   ```

### Publishing Command:
Run from the root of `/home/dhimasardinata/Dokumen/base`:
```bash
pio pkg publish --type library
```
Once published, users can install AetherIoT simply via:
```ini
lib_deps = dhimasardinata/AetherIoT
```

---

## 2. ESP-IDF Component Registry Submission

Espressif's Component Registry uses the manifest in [`idf_component.yml`](../idf_component.yml).

### Prerequisites:
1. Log in to [components.espressif.com](https://components.espressif.com/) and generate an API Token.
2. Set the token in your terminal:
   ```bash
   export IDF_COMPONENT_API_TOKEN="your_token_here"
   ```

### Publishing Command:
```bash
python -m idf_component_tools.cli upload --namespace dhimasardinata --name AetherIoT
```
Or via standard IDF CLI:
```bash
idf.py upload-component --namespace dhimasardinata --name AetherIoT
```

---

## 3. Arduino IDE Library Manager Submission

Arduino automatically indexes libraries hosted on GitHub via git release tags and [`library.properties`](../library.properties).

### Steps:
1. Push the code and create a GitHub release with tag `v1.0.0`:
   ```bash
   git tag -a v1.0.0 -m "Release v1.0.0"
   git push origin main --tags
   ```
2. Open an issue on the official [Arduino Library Registry Repository](https://github.com/arduino/library-registry/issues/new?template=new-library.md):
   - **Repository URL**: `https://github.com/dhimasardinata/AetherIoT.git`
   - **Category**: `Communication`
3. Arduino's automated bot will validate `library.properties` and index AetherIoT directly into the Arduino IDE Library Manager search engine within 24-48 hours.
