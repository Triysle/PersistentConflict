class PC_PlayerPersistenceData
{
    string m_sPlayerUID;
    int m_iTotalXP;
    int m_iCurrentRank;
    
    void PC_PlayerPersistenceData(string uid = "", int xp = 0, int rank = 0)
    {
        m_sPlayerUID = uid;
        m_iTotalXP = xp;
        m_iCurrentRank = rank;
    }
    
    // Convert to/from JSON for file storage
    static PC_PlayerPersistenceData FromJSON(JsonApiStruct data)
    {
        PC_PlayerPersistenceData result = new PC_PlayerPersistenceData();
        result.m_sPlayerUID = data.GetString("uid");
        result.m_iTotalXP = data.GetInt("xp");
        result.m_iCurrentRank = data.GetInt("rank");
        return result;
    }
    
    JsonApiStruct ToJSON()
    {
        JsonApiStruct data = new JsonApiStruct();
        data.Set("uid", m_sPlayerUID);
        data.Set("xp", m_iTotalXP);
        data.Set("rank", m_iCurrentRank);
        return data;
    }
}