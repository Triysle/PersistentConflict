[BaseContainerProps()]
class PC_PersistenceManager : SCR_BaseGameModeComponent
{
    private ref map<string, ref PC_PlayerPersistenceData> m_mPlayerDatabase;
    private const string DATABASE_FILE = "$profile:PersistentConflict_PlayerData.json";
    
    void PC_PersistenceManager(IEntityComponentSource src, IEntity ent, IEntity parent)
    {
        m_mPlayerDatabase = new map<string, ref PC_PlayerPersistenceData>();
    }
    
    override void OnPostInit(IEntity owner)
    {
        super.OnPostInit(owner);
        LoadDatabase();
        
        // Set up automatic saving every 5 minutes
        GetGame().GetCallqueue().CallLater(AutoSave, 300000, true);
    }
    
    void LoadDatabase()
    {
        FileHandle file = FileIO.OpenFile(DATABASE_FILE, FileMode.READ);
        if (!file)
        {
            Print("[PC_PersistenceManager] No existing database file found, starting fresh");
            return;
        }
        
        string jsonString;
        string line;
        
        // Read entire file
        while (file.ReadLine(line) > 0)
        {
            jsonString += line;
        }
        file.Close();
        
        if (jsonString.IsEmpty())
            return;
            
        // Parse JSON
        JsonApi jsonApi = GetGame().GetJsonApi();
        JsonApiStruct jsonData = jsonApi.LoadFromString(jsonString);
        
        if (!jsonData)
        {
            Print("[PC_PersistenceManager] Failed to parse database JSON");
            return;
        }
        
        // Load player data
        JsonApiStruct playersData = jsonData.Get("players");
        if (playersData)
        {
            for (int i = 0; i < playersData.GetCount(); i++)
            {
                JsonApiStruct playerJson = playersData.Get(i);
                PC_PlayerPersistenceData data = PC_PlayerPersistenceData.FromJSON(playerJson);
                m_mPlayerDatabase.Set(data.m_sPlayerUID, data);
            }
        }
        
        Print(string.Format("[PC_PersistenceManager] Loaded %1 player records", m_mPlayerDatabase.Count()));
    }
    
    void SaveDatabase()
    {
        JsonApi jsonApi = GetGame().GetJsonApi();
        JsonApiStruct rootData = new JsonApiStruct();
        JsonApiStruct playersArray = new JsonApiStruct();
        
        // Convert all player data to JSON
        foreach (string uid, PC_PlayerPersistenceData data : m_mPlayerDatabase)
        {
            playersArray.Set(playersArray.GetCount().ToString(), data.ToJSON());
        }
        
        rootData.Set("players", playersArray);
        rootData.Set("lastSaved", GetGame().GetWorld().GetWorldTime());
        
        string jsonString = jsonApi.SaveToString(rootData);
        
        FileHandle file = FileIO.OpenFile(DATABASE_FILE, FileMode.WRITE);
        if (file)
        {
            file.Write(jsonString);
            file.Close();
            Print("[PC_PersistenceManager] Database saved successfully");
        }
        else
        {
            Print("[PC_PersistenceManager] Failed to save database!");
        }
    }
    
    void AutoSave()
    {
        SaveDatabase();
    }
    
    PC_PlayerPersistenceData GetPlayerData(string playerUID)
    {
        PC_PlayerPersistenceData data;
        if (!m_mPlayerDatabase.Find(playerUID, data))
        {
            // Create new player entry with default values
            data = new PC_PlayerPersistenceData(playerUID, 0, 0);
            m_mPlayerDatabase.Set(playerUID, data);
        }
        return data;
    }
    
    void UpdatePlayerData(string playerUID, int totalXP, int currentRank)
    {
        PC_PlayerPersistenceData data = GetPlayerData(playerUID);
        data.m_iTotalXP = totalXP;
        data.m_iCurrentRank = currentRank;
        
        Print(string.Format("[PC_PersistenceManager] Updated %1: XP=%2, Rank=%3", playerUID, totalXP, currentRank));
    }
}