[EntityEditorProps(category: "GameScripted/GameMode", description: "Persistent Conflict Game Mode")]
class PC_ConflictGameModeClass: SCR_ConflictGameModeClass
{
}

class PC_ConflictGameMode : SCR_ConflictGameMode
{
    [Attribute("", UIWidgets.Object)]
    protected ref PC_PersistenceManager m_PersistenceManager;
    
    override void EOnInit(IEntity owner)
    {
        super.EOnInit(owner);
        
        // Get or create persistence manager
        m_PersistenceManager = PC_PersistenceManager.Cast(FindComponent(PC_PersistenceManager));
        if (!m_PersistenceManager)
        {
            Print("[PC_ConflictGameMode] Warning: No PersistenceManager component found!");
        }
    }
    
    override void OnPlayerAuditSuccess(int playerID)
    {
        super.OnPlayerAuditSuccess(playerID);
        
        // Restore player's saved XP/rank after they're fully connected
        GetGame().GetCallqueue().CallLater(RestorePlayerProgress, 1000, false, playerID);
    }
    
    override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
    {
        // Save player progress before they disconnect
        SavePlayerProgress(playerId);
        super.OnPlayerDisconnected(playerId, cause, timeout);
    }
    
    override void OnGameEnd()
    {
        // Save all players' progress when match ends
        SaveAllPlayersProgress();
        super.OnGameEnd();
    }
    
    void RestorePlayerProgress(int playerID)
    {
        if (!m_PersistenceManager)
            return;
            
        // Get player entity and XP handler
        IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
        if (!playerEntity)
        {
            Print(string.Format("[PC_ConflictGameMode] Could not find entity for player %1", playerID));
            return;
        }
        
        SCR_XPHandlerComponent xpHandler = SCR_XPHandlerComponent.Cast(playerEntity.FindComponent(SCR_XPHandlerComponent));
        if (!xpHandler)
        {
            Print(string.Format("[PC_ConflictGameMode] Could not find XP handler for player %1", playerID));
            return;
        }
        
        // Get saved progress
        string playerUID = GetGame().GetPlayerManager().GetPlayerIdentityId(playerID);
        PC_PlayerPersistenceData savedData = m_PersistenceManager.GetPlayerData(playerUID);
        
        if (savedData.m_iTotalXP > 0)
        {
            // Restore the saved XP
            xpHandler.SetPlayerXP(savedData.m_iTotalXP);
            
            Print(string.Format("[PC_ConflictGameMode] Restored %1 XP for player %2 (%3)", 
                savedData.m_iTotalXP, playerID, playerUID));
                
            // Show notification to player
            SCR_NotificationsComponent.SendToPlayer(playerID, 
                ENotification.PLAYER_PROMOTION, 
                string.Format("Progress Restored: %1 XP", savedData.m_iTotalXP));
        }
    }
    
    void SavePlayerProgress(int playerID)
    {
        if (!m_PersistenceManager)
            return;
            
        IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
        if (!playerEntity)
            return;
            
        SCR_XPHandlerComponent xpHandler = SCR_XPHandlerComponent.Cast(playerEntity.FindComponent(SCR_XPHandlerComponent));
        if (!xpHandler)
            return;
            
        string playerUID = GetGame().GetPlayerManager().GetPlayerIdentityId(playerID);
        
        // Get current XP and rank
        SCR_PlayerXPInfo xpInfo = xpHandler.GetPlayerXPInfo();
        if (xpInfo)
        {
            int currentXP = xpInfo.GetXP();
            int currentRank = xpInfo.GetRank();
            
            // Save to database
            m_PersistenceManager.UpdatePlayerData(playerUID, currentXP, currentRank);
            
            Print(string.Format("[PC_ConflictGameMode] Saved progress for player %1: %2 XP, Rank %3", 
                playerID, currentXP, currentRank));
        }
    }
    
    void SaveAllPlayersProgress()
    {
        if (!m_PersistenceManager)
            return;
            
        array<int> playerIDs = {};
        GetGame().GetPlayerManager().GetPlayers(playerIDs);
        
        foreach (int playerID : playerIDs)
        {
            SavePlayerProgress(playerID);
        }
        
        // Force save to file
        m_PersistenceManager.SaveDatabase();
    }
}