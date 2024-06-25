/*
 ESP32-S3 Memory Checker
 By Dr.Kishor Kumar
*/
#include <Arduino.h>
#include <esp_chip_info.h>
#include <esp_partition.h>
#include <esp_efuse.h>
#include <LittleFS.h>

// Global variables to store allocation results
size_t allocated_ram = 0;
size_t written_flash = 0;

void printChipInfo() {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  Serial.println("Chip Information:");
  Serial.print("  Model: ");
  switch (chip_info.model) {
    case CHIP_ESP32S3:
      Serial.println("ESP32-S3");
      break;
    default:
      Serial.println("Unknown");
      break;
  }
  Serial.print("  Cores: ");
  Serial.println(chip_info.cores);
  Serial.print("  Revision: ");
  Serial.println(chip_info.revision);
  Serial.print("  Feature: ");
  Serial.printf("0x%02X\n", chip_info.features);

  Serial.println("************************************");

  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");

  Serial.println("************************************");

  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());

  Serial.println("************************************");

  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());

  Serial.println("************************************");

  Serial.print("Chip Cores: ");
  Serial.println(ESP.getChipCores());

  Serial.println("************************************");

  uint8_t macAddress[6];
  esp_efuse_mac_get_default(macAddress);
  Serial.print("MAC Address: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(macAddress[i], HEX);
    if (i < 5) {
      Serial.print(":");
    }
  }
  Serial.println();

  Serial.println("************************************");

  uint32_t psram_size = ESP.getPsramSize();
  uint32_t psram_free_size = ESP.getFreePsram();

  Serial.println("PSRAM Information:");
  Serial.print("  Total Size: ");
  Serial.print(psram_size / (1024 * 1024));
  Serial.println(" MB");
  Serial.print("  Free Size: ");
  Serial.print(psram_free_size / (1024 * 1024));
  Serial.println(" MB");

  Serial.println("************************************");

  uint32_t total_ram = ESP.getHeapSize();
  uint32_t free_ram = ESP.getFreeHeap();

  Serial.println("Built-in RAM Information:");
  Serial.print("  Total Size: ");
  Serial.print(total_ram / 1024);
  Serial.println(" KB");
  Serial.print("  Free Size: ");
  Serial.print(free_ram / 1024);
  Serial.println(" KB");

  Serial.println("************************************");

  const esp_partition_type_t partition_types[] = {
    ESP_PARTITION_TYPE_APP,
    ESP_PARTITION_TYPE_DATA,
    ESP_PARTITION_TYPE_ANY
  };

  const char *partition_type_names[] = {
    "Application",
    "Data",
    "Any"
  };

  uint32_t highest_partition_address = 0;
  uint32_t total_partition_size = 0;

  Serial.println("Partition Information:");
  for (int i = 0; i < sizeof(partition_types) / sizeof(partition_types[0]); i++) {
    esp_partition_iterator_t it = esp_partition_find(partition_types[i], ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (it == NULL) {
      Serial.printf("No partitions of type '%s' found.\n", partition_type_names[i]);
      continue;
    }
    Serial.printf("Partitions of type '%s':\n", partition_type_names[i]);
    while (it != NULL) {
      const esp_partition_t *part = esp_partition_get(it);
      if (part == NULL) {
        Serial.println("Error: Unable to get partition information.");
        break;
      }
      Serial.print("  Label: ");
      Serial.println(part->label);
      Serial.print("    Type: ");
      Serial.print(part->type);
      Serial.print(", Subtype: ");
      Serial.println(part->subtype);
      Serial.print("    Address: 0x");
      Serial.println(part->address, HEX);
      Serial.print("    Size: ");
      Serial.print(part->size / 1024);
      Serial.println(" KB");
      Serial.print("    Encrypted: ");
      Serial.println(part->encrypted ? "Yes" : "No");

      uint32_t partition_end = part->address + part->size;
      if (partition_end > highest_partition_address) {
        highest_partition_address = partition_end;
      }

      total_partition_size += part->size;

      it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);
  }

  Serial.print("Highest Partition Address: 0x");
  Serial.println(highest_partition_address, HEX);

  Serial.print("Total Partition Size: ");
  Serial.print(total_partition_size / 1024);
  Serial.println(" KB");

  Serial.print("Inferred Total Flash Size: ");
  Serial.print(highest_partition_address / (1024 * 1024));
  Serial.println(" MB");

  Serial.println("************************************");
}

void fillRAM() {
  Serial.println("Filling RAM...");
  size_t block_size = 32768; // Start with 32KB blocks
  allocated_ram = 0;
  size_t allocation_count = 0;
  unsigned long start_time = millis();

  // First, try to allocate larger blocks
  while (block_size >= 1024) {
    void* block = ps_malloc(block_size);
    if (block != nullptr) {
      memset(block, 0xAA, block_size); // Write pattern to ensure allocation
      allocated_ram += block_size;
      allocation_count++;
    } else {
      block_size /= 2; // Reduce block size if allocation fails
    }

    // Print progress every 1MB
    if (allocated_ram % (1024 * 1024) == 0) {
      Serial.printf("Allocated %u MB\n", allocated_ram / (1024 * 1024));
    }
  }

  // Then, try to fill remaining space with 1KB blocks
  block_size = 1024;
  while (true) {
    void* block = ps_malloc(block_size);
    if (block == nullptr) {
      break; // No more memory available
    }
    memset(block, 0xAA, block_size);
    allocated_ram += block_size;
    allocation_count++;
  }

  unsigned long duration = millis() - start_time;

  Serial.printf("Allocated RAM: %u bytes (%u MB)\n", allocated_ram, allocated_ram / (1024 * 1024));
  Serial.printf("Number of allocations: %u\n", allocation_count);
  Serial.printf("Allocation speed: %.2f KB/s\n", allocated_ram / 1024.0 / (duration / 1000.0));
  
  // Get total and free PSRAM
  size_t total_psram = ESP.getPsramSize();
  size_t free_psram = ESP.getFreePsram();
  
  Serial.printf("Total PSRAM: %u bytes (%u MB)\n", total_psram, total_psram / (1024 * 1024));
  Serial.printf("Free PSRAM after allocation: %u bytes (%u MB)\n", free_psram, free_psram / (1024 * 1024));
  Serial.printf("PSRAM utilization: %.2f%%\n", 100.0 * (total_psram - free_psram) / total_psram);
}

void fillFlash() {
  Serial.println("Filling Flash...");
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    return;
  }

  size_t total_space = LittleFS.totalBytes();
  size_t used_space = LittleFS.usedBytes();
  if (total_space <= used_space) {
    Serial.println("No available space in LittleFS");
    return;
  }
  size_t available_space = total_space - used_space;

  Serial.printf("Total LittleFS space: %u bytes\n", total_space);
  Serial.printf("Available space: %u bytes\n", available_space);

  written_flash = 0;
  size_t chunk_size = (size_t)std::min<unsigned long>(32768UL, (unsigned long)available_space);

  uint8_t* buffer = (uint8_t*)ps_malloc(chunk_size);
  if (!buffer) {
    buffer = (uint8_t*)malloc(chunk_size);
  }

  if (!buffer) {
    Serial.println("Failed to allocate buffer");
    return;
  }

  Serial.printf("Using buffer size: %u bytes\n", chunk_size);
  memset(buffer, 0xBB, chunk_size);

  File file = LittleFS.open("/test_file.bin", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    free(buffer);
    return;
  }

  unsigned long startTime = millis();
  unsigned long lastPrintTime = startTime;
  const unsigned long printInterval = 1000; // Print every second

  while (written_flash < available_space) {
    if (available_space - written_flash < chunk_size) {
      chunk_size = available_space - written_flash;
    }
    size_t bytes_written = file.write(buffer, chunk_size);
    written_flash += bytes_written;

    if (bytes_written < chunk_size) {
      Serial.println("Reached end of available space or write error occurred");
      break;
    }

    unsigned long currentTime = millis();
    if (currentTime - lastPrintTime >= printInterval) {
      Serial.printf("Written %u MB\n", written_flash / (1024 * 1024));
      lastPrintTime = currentTime;
    }

    // Check for potential overflow
    if (written_flash > available_space) {
      Serial.println("Error: Written more than available space");
      break;
    }
  }

  file.close();
  free(buffer);

  unsigned long duration = millis() - startTime;
  Serial.printf("Total Written Flash: %u bytes\n", written_flash);
  if (duration > 0) {
    float avgSpeed = (float)written_flash / (float)duration * 1000.0 / 1024.0;
    Serial.printf("Average Write speed: %.2f KB/s\n", avgSpeed);
  } else {
    Serial.println("Write duration too short to calculate speed");
  }

  LittleFS.end();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }

  printChipInfo();

  Serial.println("\nCommands:");
  Serial.println("'w' - Write test (fill RAM and Flash)");
  Serial.println("'r' - Read test (report allocated RAM and written Flash)");
}

void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      case 'w':
        fillRAM();
        fillFlash();
        break;
      case 'r':
        Serial.printf("Allocated RAM: %u bytes\n", allocated_ram);
        Serial.printf("Written Flash: %u bytes\n", written_flash);
        break;
      default:
        Serial.println("Unknown command. Use 'w' to write or 'r' to read.");
    }
  }
}