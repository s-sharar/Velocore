"use client"

import { useEffect, useState } from "react"
import { Badge } from "@/components/ui/badge"
import { Wifi, WifiOff } from "lucide-react"

interface ConnectionStatusData {
  connected: boolean
  subscribed_symbols: string[]
}

export function ConnectionStatus() {
  const [status, setStatus] = useState<ConnectionStatusData>({
    connected: false,
    subscribed_symbols: [],
  })

  useEffect(() => {
    const checkStatus = async () => {
      try {
        const response = await fetch("http://localhost:18080/market/status")
        if (response.ok) {
          const data = await response.json()
          setStatus(data)
        }
      } catch (error) {
        console.error("Failed to check connection status:", error)
        setStatus({ connected: false, subscribed_symbols: [] })
      }
    }

    checkStatus()
    const interval = setInterval(checkStatus, 5000)
    return () => clearInterval(interval)
  }, [])

  return (
    <div className="px-2 py-2">
      <Badge variant={status.connected ? "default" : "destructive"} className="w-full justify-start gap-2">
        {status.connected ? <Wifi className="h-3 w-3" /> : <WifiOff className="h-3 w-3" />}
        {status.connected ? "Connected" : "Disconnected"}
      </Badge>
      {status.subscribed_symbols.length > 0 && (
        <div className="mt-2 text-xs text-muted-foreground">Subscribed: {status.subscribed_symbols.join(", ")}</div>
      )}
    </div>
  )
}
