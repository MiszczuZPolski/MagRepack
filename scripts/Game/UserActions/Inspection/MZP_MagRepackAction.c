class MZP_MagRepackAction : SCR_InspectionUserAction
{
	protected SCR_InventoryStorageManagerComponent m_InventoryManager;
	
	override bool CanBeShownScript(IEntity user)
	{		
		if (!super.CanBeShownScript(user))
			return false;
		
		if (!m_InventoryManager)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			if(!character)
				return false;
			
			m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		}
			
		if(!m_InventoryManager || !m_WeaponComponent.GetCurrentMagazine())
			return false;
		
		IEntity currentMag = m_WeaponComponent.GetCurrentMagazine().GetOwner();
		
		return HasMagsToRepack(m_InventoryManager, currentMag);
	}
	
	static bool IsNotFullMag(BaseMagazineComponent magazine)
	{
		return Math.IsInRange(magazine.GetAmmoCount(),0,magazine.GetMaxAmmoCount() - 1);
	}
	
	protected bool HasMagsToRepack(SCR_InventoryStorageManagerComponent manager, IEntity magazine)
	{		
		array<IEntity> allItems = {};
		manager.GetItems(allItems);
		int partialMagsCount = 0;
		
		ResourceName currentMagName = magazine.GetPrefabData().GetPrefabName();
		
		//find partial mags in unit inventory
		foreach (IEntity itemEntity : allItems)
		{
			BaseMagazineComponent invItem = BaseMagazineComponent.Cast(itemEntity.FindComponent(BaseMagazineComponent));
			if ( invItem )
			{
				//if mag is used then bypass
				if (invItem.IsUsed())
					continue;			
								
				string itemName = invItem.GetOwner().GetPrefabData().GetPrefabName();
				
				//only repack mags of currently equipped weapon
				if (itemName == currentMagName && IsNotFullMag(invItem))
				{
					partialMagsCount++;
				}
			}
		}
		
		return (partialMagsCount > 1);	
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (!m_WeaponComponent)
			return false;

		IEntity currentMag = m_WeaponComponent.GetCurrentMagazine().GetOwner();
		if (!currentMag)
			return false;

		InventoryItemComponent magItem = InventoryItemComponent.Cast(currentMag.FindComponent(InventoryItemComponent));
		if (!magItem)
			return false;

		UIInfo actionInfo = GetUIInfo();
		UIInfo itemInfo = magItem.GetUIInfo();
		if (actionInfo && itemInfo)
		{
			outName = string.Format("%1%2", actionInfo.GetName(), itemInfo.GetName());
			return true;
		}
		else
		{
			return false;
		}
	}
	
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		BaseMagazineComponent currentMag = m_WeaponComponent.GetCurrentMagazine();
		private int maxAmmoCount = currentMag.GetMaxAmmoCount();
		
		//find currently used mag prefab name
		ResourceName currentMagName = pOwnerEntity.GetPrefabData().GetPrefabName();

		array<IEntity> allItems = {};
		m_InventoryManager.GetItems(allItems);
		private int allAmmoCount = 0;
		
		//find partial mags in unit inventory
		foreach (IEntity item : allItems)
		{
			BaseMagazineComponent invItem = BaseMagazineComponent.Cast(item.FindComponent(BaseMagazineComponent));
			if ( invItem )
			{
				//if mag is used then bypass
				if (invItem.IsUsed())
					continue;			
								
				string itemName = invItem.GetOwner().GetPrefabData().GetPrefabName();
				
				//only repack mags of currently equipped weapon
				if (itemName == currentMagName && IsNotFullMag(invItem))
				{
					allAmmoCount = allAmmoCount + invItem.GetAmmoCount();
					delete item;
				}
			}
		}

		//add full and partial mags
		int magazinesToAdd = allAmmoCount/maxAmmoCount;
		int partialAmmoCount = Math.Mod(allAmmoCount, maxAmmoCount);
		
		for (int i; i < magazinesToAdd; i++)
		{
			m_InventoryManager.TrySpawnPrefabToStorage(currentMagName);
		}
		
		//find one mag and change ammo count for partial mag
		if (partialAmmoCount > 0)
		{
			array<IEntity> replacingPartial = {};
			m_InventoryManager.TrySpawnPrefabToStorage(currentMagName);
			m_InventoryManager.GetItems(replacingPartial);
			foreach (IEntity item : replacingPartial)
			{
				BaseMagazineComponent invItem = BaseMagazineComponent.Cast(item.FindComponent(BaseMagazineComponent));
				if ( invItem )
				{
					//if mag is used then bypass
					if (invItem.IsUsed())
						continue;
					
					string itemName = invItem.GetOwner().GetPrefabData().GetPrefabName();
					
					//change ammo count of one mag and exit foreach
					if (itemName == currentMagName)
					{
						invItem.SetAmmoCount(partialAmmoCount);
						break;
					}
				}
			}
		}
	}
};