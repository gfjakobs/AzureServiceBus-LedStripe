# AzureServiceBus-LedStripe
Reads of AzureService Bus and flash an animation on the ledstrip.

Color Code in Hex, like 0x7CFC00 for Red.
Effect: These effects are implemented: Wipe, Strobe, Metor

Format on message from service bus:
{
    "Color":  "0x7CFC00",
    "Created":  "09.02.2020 16.52.03",
    "Effect":  "Wipe",
    "Env":  "Your Env"
}
