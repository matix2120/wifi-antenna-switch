import discord
import asyncio
import websockets
import json
from discord.ext import commands

DISCORD_BOT_TOKEN = "TOKEN"
WEBSOCKET_SERVER_URL = "WS_ADDRESS"

tree = discord.app_commands.CommandTree(discord.Client(intents=discord.Intents.default()))

async def send_websocket_command(command: str):
    try:
        async with websockets.connect(WEBSOCKET_SERVER_URL) as websocket:
            await websocket.send(command)
            response = await websocket.recv()
            return json.loads(response)
    except Exception as e:
        return {"error": str(e)}

class MyClient(discord.Client):
    def __init__(self):
        super().__init__(intents=discord.Intents.default())
        self.tree = discord.app_commands.CommandTree(self)

    async def setup_hook(self):
        await self.tree.sync()

client = MyClient()

@client.event
async def on_ready():
    print(f"Logged in as {client.user}")

@client.tree.command(name="off", description="Turn off the antenna")
async def off_command(interaction: discord.Interaction):
    response = await send_websocket_command("0")
    if isinstance(response, dict) and "ant" in response:
        await interaction.response.send_message("Anteny odłączone.")
    else:
        await interaction.response.send_message(f"Błąd: {response}")

@client.tree.command(name="antena", description="Wybierz antenę 1-4")
@discord.app_commands.describe(number="Antenna number (1-4)")
async def antena(interaction: discord.Interaction, number: int):
    if number not in [1, 2, 3, 4]:
        await interaction.response.send_message("Niepoprawny numer anteny. Wybierz 1 do 4.", ephemeral=True)
        return
    response = await send_websocket_command(str(number))
    if isinstance(response, dict) and "ant" in response:
        await interaction.response.send_message(f"Przełączono na antenę {response['ant']}.")
    else:
        await interaction.response.send_message(f"Błąd: {response}")

@client.tree.command(name="jakie-anteny", description="Wyświetl listę anten")
async def anteny(interaction: discord.Interaction):
    response_names = await send_websocket_command("getNames")
    response_current = await send_websocket_command("getCurrentAntenna")
    if isinstance(response_names, list) and isinstance(response_current, dict) and "ant" in response_current:
        current_antenna = response_current.get("ant")
        formatted = "\n".join(f"{i+1}. {name} {'(PODŁĄCZONA)' if i+1 == int(current_antenna) else ''}" for i, name in enumerate(response_names))
        await interaction.response.send_message(f"Dostępne anteny:\n{formatted}")
    else:
        await interaction.response.send_message(f"Błąd: {response_names if isinstance(response_names, dict) else response_current}")

if __name__ == "__main__":
    client.run(DISCORD_BOT_TOKEN)
