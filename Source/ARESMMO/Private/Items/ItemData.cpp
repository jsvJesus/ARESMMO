#include "ARESMMO/Public/Items/ItemData.h"
#include "ARESMMO/Public/Items/ItemTypes.h"

namespace ItemDB
{
	// Путь к DataTable в контенте. Позже поменяешь под своё расположение.
	static const TCHAR* ItemsDataTablePath = TEXT("/Game/ARESMMO/DataTable/DT_Items.DT_Items");

	static UDataTable* G_ItemsTable = nullptr;

	/** Ленивая загрузка DataTable по пути */
	ARESMMO_API UDataTable* GetItemsDataTable()
	{
		if (!G_ItemsTable)
		{
			G_ItemsTable = LoadObject<UDataTable>(nullptr, ItemsDataTablePath);
			if (!G_ItemsTable)
			{
				UE_LOG(LogTemp, Error, TEXT("ItemDB: Failed to load DataTable at path: %s"), ItemsDataTablePath);
			}
		}
		return G_ItemsTable;
	}

	/** Найти строку по ItemID */
	ARESMMO_API const FItemBaseRow* GetItemByID(int32 ItemID)
	{
		if (UDataTable* Table = GetItemsDataTable())
		{
			static const FString Context = TEXT("ItemDB::GetItemByID");
			TArray<FItemBaseRow*> AllRows;
			Table->GetAllRows<FItemBaseRow>(Context, AllRows);

			for (FItemBaseRow* Row : AllRows)
			{
				if (Row && Row->ItemID == ItemID)
				{
					return Row;
				}
			}
		}
		return nullptr;
	}
}