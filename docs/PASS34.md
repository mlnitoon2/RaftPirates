# Pass 34

Added portable store catalog and currency purchase response adapters to RaftPirates. These use existing catalog and purchase-response processing; they do not implement billing or original message ABI. The currency adapter is not wired to a newly inferred wire command. Store adapter returns available IDs, not a popup. Regression assertions added to session tests; not executed in this pass. Hydra companion changes affect text viewport and AE menu rendering. Remaining game handlers and live Android validation are open. Coverage remains the last compiled Pass 33 measurement.
