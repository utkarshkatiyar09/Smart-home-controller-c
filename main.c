#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 50
#define MAX_RULES 5

// Structure for sensor data
typedef struct {
    float temperature;
    int ambientLight;
    int occupancy;
} SensorData;

// Structure for schedule
typedef struct Schedule {
    int startHour;
    int endHour;
    int status;

    struct Schedule *next;
} Schedule;

// Structure for smart devices
typedef struct {
    int id;
    char name[MAX_NAME];
    char type[MAX_NAME];

    int status;
    float powerRating;

    float energyUsed;

    int manualOverride;

    Schedule *schedule;
} Device;

// Function pointer definitions
typedef int (*ConditionFunction)(SensorData, float);
typedef void (*ActionFunction)(Device *);

// Structure for rules
typedef struct {
    int conditionCode;
    char description[MAX_NAME];

    float threshold;

    ConditionFunction condition;
    ActionFunction action;
} Rule;


// Function declarations
void addDevice(Device **devices, int *count);
void displayDevices(Device *devices, int count);

void addSchedule(Device *devices, int count);

void readSensors(SensorData *sensor);

void evaluateRules(
    Device *devices,
    int count,
    SensorData sensor,
    Rule rules[],
    int ruleCount
);

void applySchedules(Device *devices, int count, int hour);

void manualControl(Device *devices, int count);

void resetOverride(Device *devices, int count);

void energyReport(Device *devices, int count);

void whatIfScenario(
    Device *devices,
    int count,
    SensorData sensor,
    Rule rules[],
    int ruleCount
);

void freeMemory(Device *devices, int count);


// Condition functions

int highTemperature(SensorData s, float threshold) {
    return s.temperature > threshold && s.occupancy;
}

int lowLight(SensorData s, float threshold) {
    return s.ambientLight < threshold && s.occupancy;
}

int moderateTemperature(SensorData s, float threshold) {
    return s.temperature > threshold && s.occupancy;
}


// Action functions

void turnON(Device *d) {
    d->status = 1;
}

void turnOFF(Device *d) {
    d->status = 0;
}


// Add device dynamically
void addDevice(Device **devices, int *count) {

    Device *temp;

    temp = (Device *)realloc(
        *devices,
        (size_t)(*count + 1) * sizeof(Device)
    );

    if (temp == NULL) {
        printf("\nMemory allocation failed!\n");
        return;
    }

    *devices = temp;

    Device *d = &(*devices)[*count];

    d->id = *count + 1;

    printf("\nEnter Device Name: ");
    scanf(" %49[^\n]", d->name);

    printf("Enter Device Type (AC/LIGHT/FAN): ");
    scanf(" %49s", d->type);

    printf("Enter Power Rating (Watts): ");
    scanf("%f", &d->powerRating);

    d->status = 0;
    d->energyUsed = 0;
    d->manualOverride = 0;
    d->schedule = NULL;

    (*count)++;

    printf("\nDevice Added Successfully!\n");
}


// Display devices
void displayDevices(Device *devices, int count) {

    int i;

    if (count == 0) {
        printf("\nNo devices available!\n");
        return;
    }

    printf("\n========== SMART HOME DEVICES ==========\n");

    for (i = 0; i < count; i++) {

        printf("\nID: %d", devices[i].id);

        printf("\nName: %s", devices[i].name);

        printf("\nType: %s", devices[i].type);

        printf("\nPower: %.2f W", devices[i].powerRating);

        printf("\nStatus: %s",
               devices[i].status ? "ON" : "OFF");

        printf("\nEnergy Used: %.3f kWh",
               devices[i].energyUsed);

        printf("\nManual Override: %s\n",
               devices[i].manualOverride ? "YES" : "NO");
    }
}


// Add schedule using linked list
void addSchedule(Device *devices, int count) {

    int id;

    Schedule *newNode;

    printf("\nEnter Device ID: ");
    scanf("%d", &id);

    if (id < 1 || id > count) {
        printf("\nInvalid Device ID!\n");
        return;
    }

    newNode = (Schedule *)malloc(sizeof(Schedule));

    if (newNode == NULL) {
        printf("\nMemory allocation failed!\n");
        return;
    }

    printf("Enter Start Hour (0-23): ");
    scanf("%d", &newNode->startHour);

    printf("Enter End Hour (1-24): ");
    scanf("%d", &newNode->endHour);

    printf("Enter Status (1=ON, 0=OFF): ");
    scanf("%d", &newNode->status);

    if (newNode->startHour < 0 ||
        newNode->startHour > 23 ||
        newNode->endHour < 1 ||
        newNode->endHour > 24 ||
        newNode->startHour >= newNode->endHour ||
        (newNode->status != 0 && newNode->status != 1)) {

        printf("\nInvalid Schedule!\n");
        free(newNode);
        return;
    }

    newNode->next = devices[id - 1].schedule;

    devices[id - 1].schedule = newNode;

    printf("\nSchedule Added Successfully!\n");
}


// Read sensor inputs
void readSensors(SensorData *sensor) {

    printf("\nEnter Current Temperature (C): ");
    scanf("%f", &sensor->temperature);

    printf("Enter Ambient Light Level (0-100): ");
    scanf("%d", &sensor->ambientLight);

    printf("Enter Occupancy (1=Present, 0=Absent): ");
    scanf("%d", &sensor->occupancy);
}


// Apply schedules
void applySchedules(Device *devices, int count, int hour) {

    int i;

    for (i = 0; i < count; i++) {

        Schedule *ptr = devices[i].schedule;

        while (ptr != NULL) {

            if (hour >= ptr->startHour &&
                hour < ptr->endHour) {

                devices[i].status = ptr->status;
            }

            ptr = ptr->next;
        }
    }
}


// Evaluate rule engine
void evaluateRules(
    Device *devices,
    int count,
    SensorData sensor,
    Rule rules[],
    int ruleCount
) {

    int i, j;

    // Apply automatic rules
    for (i = 0; i < count; i++) {

        if (devices[i].manualOverride)
            continue;

        for (j = 0; j < ruleCount; j++) {

            int match = 0;

            if (rules[j].conditionCode == 1 &&
                strcmp(devices[i].type, "AC") == 0) {

                match = 1;
            }

            else if (rules[j].conditionCode == 2 &&
                     strcmp(devices[i].type, "LIGHT") == 0) {

                match = 1;
            }

            else if (rules[j].conditionCode == 3 &&
                     strcmp(devices[i].type, "FAN") == 0) {

                match = 1;
            }

            if (match &&
                rules[j].condition(sensor, rules[j].threshold)) {

                rules[j].action(&devices[i]);

            }
        }
    }

    printf("\nAutomatic Rules Evaluated Successfully!\n");
}


// Manual device control
void manualControl(Device *devices, int count) {

    int id, choice;

    printf("\nEnter Device ID: ");
    scanf("%d", &id);

    if (id < 1 || id > count) {
        printf("\nInvalid Device ID!\n");
        return;
    }

    printf("\n1. Turn ON");
    printf("\n2. Turn OFF");

    printf("\nEnter Choice: ");
    scanf("%d", &choice);

    if (choice == 1) {

        devices[id - 1].status = 1;
        devices[id - 1].manualOverride = 1;

    }

    else if (choice == 2) {

        devices[id - 1].status = 0;
        devices[id - 1].manualOverride = 1;

    }

    else {
        printf("\nInvalid Choice!\n");
        return;
    }

    printf("\nManual Control Applied Successfully!\n");
}


// Reset manual override
void resetOverride(Device *devices, int count) {

    int id;

    printf("\nEnter Device ID: ");
    scanf("%d", &id);

    if (id < 1 || id > count) {
        printf("\nInvalid Device ID!\n");
        return;
    }

    devices[id - 1].manualOverride = 0;

    printf("\nManual Override Reset Successfully!\n");
}


// Energy report
void energyReport(Device *devices, int count) {

    int i;

    float totalEnergy = 0;

    printf("\n========== ENERGY REPORT ==========\n");

    for (i = 0; i < count; i++) {

        printf("\nDevice: %s", devices[i].name);

        printf("\nEnergy Used: %.3f kWh\n",
               devices[i].energyUsed);

        totalEnergy += devices[i].energyUsed;
    }

    printf("\nTotal Energy Consumption: %.3f kWh\n",
           totalEnergy);
}


// What-if scenario
void whatIfScenario(
    Device *devices,
    int count,
    SensorData sensor,
    Rule rules[],
    int ruleCount
) {

    int i, j;

    SensorData simulated = sensor;

    simulated.temperature += 2.0;

    printf("\n========== WHAT-IF SCENARIO ==========\n");

    printf("\nOriginal Temperature: %.2f C",
           sensor.temperature);

    printf("\nSimulated Temperature: %.2f C\n",
           simulated.temperature);

    for (i = 0; i < count; i++) {

        int predictedStatus = devices[i].status;

        if (devices[i].manualOverride) {
            printf("\n%s: Manual override active. Status unchanged.",
                   devices[i].name);
            continue;
        }

        for (j = 0; j < ruleCount; j++) {

            int match = 0;

            if (rules[j].conditionCode == 1 &&
                strcmp(devices[i].type, "AC") == 0) {

                match = 1;
            }

            else if (rules[j].conditionCode == 2 &&
                     strcmp(devices[i].type, "LIGHT") == 0) {

                match = 1;
            }

            else if (rules[j].conditionCode == 3 &&
                     strcmp(devices[i].type, "FAN") == 0) {

                match = 1;
            }

            if (match &&
                rules[j].condition(simulated, rules[j].threshold)) {

                // Determine predicted action
                if (rules[j].conditionCode == 2) {
                    predictedStatus = 1;
                }
                else {
                    predictedStatus = 1;
                }
            }
        }

        printf("\nDevice: %s", devices[i].name);

        printf("\nPredicted Status: %s\n",
               predictedStatus ? "ON" : "OFF");
    }

    printf("\nScenario Completed!\n");
}


// Free allocated memory
void freeMemory(Device *devices, int count) {

    int i;

    for (i = 0; i < count; i++) {

        Schedule *ptr = devices[i].schedule;

        while (ptr != NULL) {

            Schedule *temp = ptr;

            ptr = ptr->next;

            free(temp);
        }
    }

    free(devices);
}


// Main function
int main() {

    Device *devices = NULL;

    int count = 0;

    int choice;

    int currentHour = 0;

    SensorData sensor = {0, 100, 0};

    // Define rules
    Rule rules[MAX_RULES] = {

        {1, "AC Temperature Rule",
         25.0, highTemperature, turnON},

        {2, "Light Darkness Rule",
         40.0, lowLight, turnON},

        {3, "Fan Temperature Rule",
         30.0, moderateTemperature, turnON}
    };

    int ruleCount = 3;

    do {

        printf("\n\n====================================");
        printf("\n     SMART HOME AUTOMATION SYSTEM");
        printf("\n====================================");

        printf("\n1. Add Device");

        printf("\n2. Display Devices");

        printf("\n3. Add Device Schedule");

        printf("\n4. Evaluate Automation Rules");

        printf("\n5. Manual Device Control");

        printf("\n6. Reset Manual Override");

        printf("\n7. Energy Consumption Report");

        printf("\n8. What-If Scenario (+2 C)");

        printf("\n9. Exit");

        printf("\n====================================");

        printf("\nEnter Your Choice: ");

        scanf("%d", &choice);

        switch (choice) {

            case 1:

                addDevice(&devices, &count);

                break;


            case 2:

                displayDevices(devices, count);

                break;


            case 3:

                addSchedule(devices, count);

                break;


            case 4:

                if (count == 0) {
                    printf("\nPlease Add Devices First!\n");
                    break;
                }

                readSensors(&sensor);

                printf("\nEnter Current Hour (0-23): ");
                scanf("%d", &currentHour);

                if (currentHour < 0 || currentHour > 23) {
                    printf("\nInvalid Hour!\n");
                    break;
                }

                evaluateRules(
                    devices,
                    count,
                    sensor,
                    rules,
                    ruleCount
                );

                // Schedules have priority over automatic rules
                applySchedules(devices, count, currentHour);

                // Calculate energy for one simulated hour
                for (int i = 0; i < count; i++) {

                    if (devices[i].status) {

                        devices[i].energyUsed +=
                        devices[i].powerRating / 1000.0;
                    }
                }

                printf("\nAutomation Cycle Completed!\n");

                displayDevices(devices, count);

                break;


            case 5:

                manualControl(devices, count);

                break;


            case 6:

                resetOverride(devices, count);

                break;


            case 7:

                energyReport(devices, count);

                break;


            case 8:

                if (count == 0) {
                    printf("\nPlease Add Devices First!\n");
                    break;
                }

                whatIfScenario(
                    devices,
                    count,
                    sensor,
                    rules,
                    ruleCount
                );

                break;


            case 9:

                printf("\nExiting Smart Home System...\n");

                break;


            default:

                printf("\nInvalid Choice! Try Again.\n");
        }

    } while (choice != 9);


    // Release dynamically allocated memory
    freeMemory(devices, count);

    printf("\nMemory Released Successfully!\n");

    return 0;
}