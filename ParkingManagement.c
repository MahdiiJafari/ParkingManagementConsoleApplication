#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

typedef struct
{
    char space_code[7];
    char license_plate[7];
    char vehicle_model[50];
    char usage_duration[4];
    char status[12];
} Vehicle;

typedef struct
{
    Vehicle spaces[5];
    int occupied;
} Floor;

Floor parking[4];
int occupied[4][5] = {0};

int admin_index = -1;
char files[3][10] = {"file1.csv", "file2.csv", "file3.csv"};
char username[3][8] = {"admin", "resalat", "hemmat"};
char password[3][8] = {"1234", "1212", "8659"};

int login();
void load_data();
void reset_parking_data();
void display_parking_status();
void add_vehicle();
int validate_vehicle_type(char *type);
int validate_license_plate(char *license_plate);
int validate_duration_format(char *duration);
void save_to_file(Vehicle vehicle);
void generate_entry_receipt(Vehicle vehicle);
void remove_vehicle();
void print_history();
void display_other_parking_status();
void search_vehicle();

int main()
{
    srand(time(0));
    login();
    while (1)
    {
        printf("\n\033[1m--- Main Menu ---\033[0m\n");
        display_parking_status();
        printf("\033[33m1. Add Vehicle\n");
        printf("2. Remove Vehicle\n");
        printf("3. Vehicle Search\n");
        printf("4. History List\n");
        printf("5. Other Parkings Status\n");
        printf("\033[31m6. Exit Application\n");
        printf("7. Log Out\033[0m\n");
        printf("\033[1mEnter your choice: \033[0m");

        int choice;
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            add_vehicle();
            break;
        case 2:
            remove_vehicle();
            break;
        case 3:
            search_vehicle();
            break;
        case 4:
            print_history();
            break;
        case 5:
            display_other_parking_status();
            break;
        case 6:
            printf("Exiting application.\n");
            return 0;
        case 7:
            printf("Signing out.\n");
            admin_index = -1;
            reset_parking_data();
            system("cls");
            login();
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    }
    return 0;
}

int login()
{
    char enteredUsername[50];
    char enteredPassword[50];

    do
    {
        printf("\n--- Login ---\n");
        printf("Username: ");
        scanf("%49s", enteredUsername);
        printf("Password: ");
        scanf("%49s", enteredPassword);
        for (int i = 0; i < 3; i++)
        {
            if (strcmp(username[i], enteredUsername) == 0 &&
                strcmp(password[i], enteredPassword) == 0)
            {
                admin_index = i;
                load_data();
                return 1;
            }
        }
        printf("\033[31mInvalid username or password. Please try again.\033[0m\n");
    } while (1);
}

void add_vehicle()
{
    char type[11], plate[20], model[50], duration[4];
    int floor_index, spot_index = -1;

    printf("Enter type (motorcycle, sedan, van): ");
    scanf("%10s", type);

    floor_index = validate_vehicle_type(type);

    if (floor_index == -1)
    {
        printf("Invalid vehicle type.\n");
        return;
    }
    if (floor_index == -2)
    {
        printf("\033[31m"
               "No available spots on this floor.\n"
               "\033[0m");
        return;
    }
    int spot_found = 0;
    if (parking[floor_index].occupied < 5)
    {
        while (!spot_found)
        {
            int spot = rand() % 5;
            if (occupied[floor_index][spot] == 0)
            {
                spot_index = spot;
                spot_found = 1;
            }
        }
    }

    if (spot_index == -1)
    {
        printf("\033[31m"
               "No available spots on this floor.\n"
               "\033[0m");
        return;
    }

    printf("Enter license plate (6 characters, 00X000): ");
    scanf("%s", plate);
    while (getchar() != '\n')
        ;
    if (validate_license_plate(plate) == 0)
    {
        printf("\033[31m"
               "License plate already exists.\n"
               "\033[0m");
        return;
    }
    if (validate_license_plate(plate) == 2)
    {
        printf("\033[31m"
               "Invalid license plate format.\n"
               "\033[0m");
        return;
    }

    printf("Enter vehicle model: ");
    fgets(model, 50, stdin);
    model[strcspn(model, "\n")] = '\0';
    printf("Enter usage duration (e.g., 3h): ");
    scanf("%3s", duration);
    if (!validate_duration_format(duration))
    {
        printf("\033[31m"
               "Invalid duration format.\n"
               "\033[0m");
        return;
    }
    char space_code[7];
    sprintf(space_code, "f%d_%02d", floor_index + 1, spot_index + 1);

    Vehicle new_vehicle = {
        .space_code = "",
        .license_plate = "",
        .vehicle_model = "",
        .usage_duration = "",
        .status = "checked_in"};
    strcpy(new_vehicle.space_code, space_code);
    strcpy(new_vehicle.license_plate, plate);
    strcpy(new_vehicle.vehicle_model, model);
    strcpy(new_vehicle.usage_duration, duration);

    parking[floor_index].spaces[spot_index] = new_vehicle;
    occupied[floor_index][spot_index] = 1;
    parking[floor_index].occupied++;

    save_to_file(new_vehicle);
    generate_entry_receipt(new_vehicle);
}

int validate_vehicle_type(char *type)
{
    if (strcmp(type, "motorcycle") == 0)
        return 0;
    if (strcmp(type, "sedan") == 0)
    {

        if (parking[2].occupied < 5 || parking[1].occupied < 5)
        {
            int floor = rand() % 2 + 1;
            if (parking[floor].occupied != 5)
            {
                return floor;
            }
            else
            {
                if (floor == 1)
                {
                    return 2;
                }
                else if (floor == 2)
                {
                    return 1;
                }
            }
        }
        return -2;
    }
    if (strcmp(type, "van") == 0)
        return 3;
    return -1;
}

int validate_license_plate(char *license_plate)
{
    FILE *file = fopen(files[admin_index], "r");

    char plate[7], status[12];
    int found = 0;

    if (strlen(license_plate) != 6 
    || !isalpha(*(license_plate + 2))
    || !isdigit(*(license_plate)) 
    || !isdigit(*(license_plate + 1)) 
    || !isdigit(*(license_plate + 3)) 
    || !isdigit(*(license_plate + 4)) 
    || !isdigit(*(license_plate + 5)))
    {
        return 2;
    }
    while (fscanf(file, "%*6[^,],%6[^,],%*49[^,],%*3[^,],%11s\n", plate, status) == 2)
    {
        if (strcmp(plate, license_plate) == 0 && strcmp(status, "checked_in") == 0)
        {
            found = 1;
            break;
        }
    }

    fclose(file);
    if (found)
    {
        return 0;
    }
    else
        return 1;
}
int validate_duration_format(char *duration)
{
    if (strlen(duration) == 3 && (*(duration + 2)) == 'h' && isdigit(*(duration)) && isdigit(*(duration + 1)))
    {
        return 1;
    }
    else if (strlen(duration) == 2 && (*(duration + 1)) == 'h' && isdigit(*(duration)))
    {
        return 1;
    }
    else
        return 0;
}
void remove_vehicle()
{
    char license_plate[7];
    printf("Enter license plate: ");
    scanf("%6s", license_plate);

    FILE *file = fopen(files[admin_index], "r");
    if (!file)
    {
        printf("Error opening file.\n");
        return;
    }

    FILE *temp = fopen("temp.csv", "w");
    if (!temp)
    {
        fclose(file);
        printf("Error creating temporary file.\n");
        return;
    }

    char space_code[7], plate[7], model[50], duration[4], status[12];
    int found = 0;

    while (fscanf(file, "%6[^,],%6[^,],%49[^,],%3[^,],%11s\n",
                  space_code, plate, model, duration, status) == 5)
    {
        if (strcmp(plate, license_plate) == 0 && strcmp(status, "checked_in") == 0)
        {
            strcpy(status, "checked_out");
            found = 1;

            int floor_num = space_code[1] - '1';
            int spot_num = atoi(&space_code[3]) - 1;
            if (floor_num >= 0 && floor_num < 4 && spot_num >= 0 && spot_num < 5)
            {
                occupied[floor_num][spot_num] = 0;
                parking[floor_num].occupied--;
            }
        }
        else if (strcmp(plate, license_plate) == 0 && strcmp(status, "checked_out") == 0)
        {
            found = 2;
        }
        fprintf(temp, "%s,%s,%s,%s,%s\n", space_code, plate, model, duration, status);
    }

    fclose(file);
    fclose(temp);
    remove(files[admin_index]);
    rename("temp.csv", files[admin_index]);
    if (found == 1)
    {
        printf("Vehicle checked out successfully.\n");
    }
    else if (found == 2)
    {
        printf("Vehicle already checked out.\n");
    }
    else
    {
        printf("Vehicle not found.\n");
    }
}

void display_parking_status()
{
    for (int f = 0; f < 4; f++)
    {
        int available = 0;
        for (int s = 0; s < 5; s++)
        {
            available += !occupied[f][s];
        }
        if (available == 0)
        {
            printf("\033[31mFloor %d: %d/5 available.  \033[0m", f + 1, available);
        }
        else
        {
            printf("\033[34mFloor %d: %d/5 available.  \033[0m", f + 1, available);
        }
        for (int j = 0; j < 5; j++)
        {
            if (occupied[f][j])
            {
                printf("| \033[31m#\033[0m ");
            }
            else
            {
                printf("| \033[32m_\033[0m ");
            }
        }
        printf("|\n");
    }
}

void save_to_file(Vehicle vehicle)
{
    FILE *file = fopen(files[admin_index], "a");
    if (!file)
    {
        printf("Error saving data.\n");
        return;
    }
    fprintf(file, "%s,%s,%s,%s,%s\n",
            vehicle.space_code,
            vehicle.license_plate,
            vehicle.vehicle_model,
            vehicle.usage_duration,
            vehicle.status);
    fclose(file);
}

void generate_entry_receipt(Vehicle vehicle)
{
    printf("\n\033[32m--- Entry Receipt ---\033[0m\n");
    printf("\033[36mSpace Code:\033[0m    %s\n", vehicle.space_code);
    printf("\033[36mLicense Plate:\033[0m %s\n", vehicle.license_plate);
    printf("\033[36mVehicle Model:\033[0m %s\n", vehicle.vehicle_model);
    printf("\033[36mDuration: \033[0m     %s\n", vehicle.usage_duration);
    printf("\033[36mStatus: \033[0m       %s\n", vehicle.status);
    sleep(5);
}
void print_history()
{
    FILE *file = fopen(files[admin_index], "r");
    if (!file)
    {
        printf("Error opening history.\n");
        return;
    }
    Vehicle history[100];
    int count = 0;
    while (fscanf(file, "%6[^,],%6[^,],%49[^,],%3[^,],%11s\n",
                  history[count].space_code,
                  history[count].license_plate,
                  history[count].vehicle_model,
                  history[count].usage_duration,
                  history[count].status) == 5)
    {
        count++;
    }
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (strcmp(history[j].space_code, history[j + 1].space_code) > 0)
            {
                Vehicle temp = history[j];
                history[j] = history[j + 1];
                history[j + 1] = temp;
            }
        }
    }
    printf("\n--- Parking History (Sorted by Space Code) ---\n");
    printf("space |license |vehicle model         |Durat|status\n");
    printf("code  |plate   |                      |ion  |\n");
    printf("------+--------+----------------------+-----+------------\n");
    //      f2_01 | 12w333 | 206 tip 2 1390       | 4h  | checked_in
    for (int i = 0; i < count; i++)
    {
        printf("%s | %s | %-20s | %-3s | %s \n",
               history[i].space_code,
               history[i].license_plate,
               history[i].vehicle_model,
               history[i].usage_duration,
               history[i].status);
    }
    fclose(file);
    sleep(4);
}
void search_vehicle()
{
    printf("\na. Search by License Plate\nb. Search by Vehicle  Model\nc. Search by Duration\nEnter your choice: ");
    char space_code[7], plate[7], model[50], duration[4], status[12];
    int found = 0;
    char choice;
    scanf(" %c", &choice);
    FILE *file = fopen(files[admin_index], "r");
    if (!file)
    {
        printf("Error opening data.\n");
        return;
    }
    switch (choice)
    {
    case 'a':
    {
        char license_plate[7];
        printf("Enter License Plate: ");
        scanf("%6s", license_plate);

        while (fscanf(file, "%6[^,],%6[^,],%49[^,],%3[^,],%11s\n",
                      space_code, plate, model, duration, status) == 5)
        {
            if (strcmp(plate, license_plate) == 0 && strcmp(status, "checked_in") == 0)
            {
                found = 1;
                printf("\n\033[32m--- Vehicle Found ---\033[0m\n");
                printf("Space Code: %s\n", space_code);
                printf("Model:      %s\n", model);
                printf("Duration:   %s\n", duration);
                printf("Status:     %s\n", status);
            }
            else if (strcmp(plate, license_plate) == 0 && strcmp(status, "checked_out") == 0)
            {
                found = 1;
                printf("\n\033[32m--- Vehicle Found (already checked out) ---\033[0m\n");
                printf("Space Code: %s\n", space_code);
                printf("Model:      %s\n", model);
                printf("Duration:   %s\n", duration);
                printf("Status:     %s\n", status);
            }
        }
        if (!found)
        {
            printf("\033[31mVehicle not found.\033[0m\n");
        }
    }
    break;
    case 'b':
    {
        char vehicle_model[50];
        printf("Enter Vehicle  Model: ");
        scanf(" %49[^\n]", vehicle_model);

        while (fscanf(file, "%6[^,],%6[^,],%49[^,],%3[^,],%11s\n",
                      space_code, plate, model, duration, status) == 5)
        {
            if (strcmp(model, vehicle_model) == 0 && strcmp(status, "checked_in") == 0)
            {
                found = 1;
                printf("\n\033[32m--- Vehicle Found ---\033[0m\n");
                printf("Space Code: %s\n", space_code);
                printf("Model:      %s\n", model);
                printf("Duration:   %s\n", duration);
                printf("Status:     %s\n", status);
            }
            else if (strcmp(model, vehicle_model) == 0 && strcmp(status, "checked_out") == 0)
            {
                found = 1;
                printf("\n\033[32m--- Vehicle Found (already checked out) ---\033[0m\n");
                printf("Space Code: %s\n", space_code);
                printf("Model:      %s\n", model);
                printf("Duration:   %s\n", duration);
                printf("Status:     %s\n", status);
            }
        }
        if (!found)
        {
            printf("\033[31mVehicle not found.\033[0m\n");
        }
    }
    break;
    case 'c':
    {
        char vehicle_duration[4];
        printf("Enter Duration: ");
        scanf("%4s", vehicle_duration);

        while (fscanf(file, "%6[^,],%6[^,],%49[^,],%3[^,],%11s\n",
                      space_code, plate, model, duration, status) == 5)
        {
            if (strcmp(duration, vehicle_duration) == 0 && strcmp(status, "checked_in") == 0)
            {
                found = 1;
                printf("\n\033[32m--- Vehicle Found ---\033[0m\n");
                printf("Space Code: %s\n", space_code);
                printf("Model:      %s\n", model);
                printf("Duration:   %s\n", duration);
                printf("Status:     %s\n", status);
            }
            else if (strcmp(duration, vehicle_duration) == 0 && strcmp(status, "checked_out") == 0)
            {
                found = 1;
                printf("\n\033[32m--- Vehicle Found (already checked out) ---\033[0m\n");
                printf("Space Code: %s\n", space_code);
                printf("Model:      %s\n", model);
                printf("Duration:   %s\n", duration);
                printf("Status:     %s\n", status);
            }
        }
        if (!found)
        {
            printf("\033[31mNo vehicle found with the specified details.\033[0m\n");
        }
    }
    break;
    default:
        break;
    }

    fclose(file);
}
void load_data()
{
    FILE *file = fopen(files[admin_index], "r");
    if (!file)
    {
        FILE *file = fopen(files[admin_index], "w");
        fclose(file);
        return;
    }
    char space_code[7], plate[7], model[50], duration[4], status[12];
    while (fscanf(file, "%6[^,],%6[^,],%49[^,],%3[^,],%11s\n",
                  space_code, plate, model, duration, status) == 5)
    {
        if (strcmp(status, "checked_in") == 0)
        {
            int floor_index = space_code[1] - '0' - 1;
            int space_index = space_code[4] - '0' - 1;
            occupied[floor_index][space_index] = 1;
            parking[floor_index].occupied++;
        }
    }
    fclose(file);
}
void display_other_parking_status()
{
    for (int i = 0; i < 3; i++)
    {
        if (i != admin_index)
        {
            FILE *filet = fopen(files[i], "r");
            if (!filet)
            {
                FILE *filet = fopen(files[i], "w");
            }
            int full[4] = {0};
            char space_code[7], plate[7], model[50], duration[4], status[12];
            while (fscanf(filet, "%6[^,],%6[^,],%49[^,],%3[^,],%11s\n",
                          space_code, plate, model, duration, status) == 5)
            {
                if (strcmp(status, "checked_in") == 0)
                {
                    int floor_index = space_code[1] - '0' - 1;
                    full[floor_index]++;
                }
            }
            fclose(filet);
            printf("%s parking status :\n", username[i]);
            for (int f = 0; f < 4; f++)
            {
                int available = 5 - full[f];

                if (available == 0)
                {
                    printf("\033[31mFloor %d: %d/5 available.  \033[0m\n", f + 1, available);
                }
                else
                {
                    printf("\033[34mFloor %d: %d/5 available.  \033[0m\n", f + 1, available);
                }
            }
        }
    }
}

void reset_parking_data()
{
    for (int f = 0; f < 4; f++)
    {
        parking[f].occupied = 0;
        for (int s = 0; s < 5; s++)
        {
            occupied[f][s] = 0;
        }
    }
}
