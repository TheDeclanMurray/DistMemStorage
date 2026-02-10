

// assumption
// storages created and attached before client activity

// for each client/storage
void* listenerThread(void* args);

void attachNewStorageNode(char* nodeHashId, char* nodeAddress);

bool isHashInMyRange(int hash);

bool set(char* key, char* value);

bool get(char* key);
