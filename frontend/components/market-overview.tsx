"use client"

import { useEffect, useState } from "react"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Badge } from "@/components/ui/badge"
import { TrendingUp } from "lucide-react"

interface MarketTick {
  symbol: string
  type: string
  timestamp: number
  trade_price?: number
  trade_size?: number
  bid_price?: number
  ask_price?: number
  bid_size?: number
  ask_size?: number
}

export function MarketOverview() {
  const [ticks, setTicks] = useState<MarketTick[]>([])
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    const fetchMarketData = async () => {
      try {
        const response = await fetch("http://localhost:18080/market/data")
        if (response.ok) {
          const data = await response.json()
          setTicks(data.ticks || [])
        }
      } catch (error) {
        console.error("Failed to fetch market data:", error)
      } finally {
        setLoading(false)
      }
    }

    fetchMarketData()
    const interval = setInterval(fetchMarketData, 2000)
    return () => clearInterval(interval)
  }, [])

  return (
    <Card>
      <CardHeader>
        <CardTitle className="flex items-center gap-2">
          <TrendingUp className="h-5 w-5" />
          Live Market Ticker
        </CardTitle>
      </CardHeader>
      <CardContent>
        {loading ? (
          <div className="space-y-2">
            {[...Array(3)].map((_, i) => (
              <div key={i} className="flex justify-between items-center p-2 border rounded">
                <div className="h-4 w-16 bg-muted animate-pulse rounded" />
                <div className="h-4 w-20 bg-muted animate-pulse rounded" />
                <div className="h-4 w-12 bg-muted animate-pulse rounded" />
              </div>
            ))}
          </div>
        ) : ticks.length === 0 ? (
          <div className="text-center text-muted-foreground py-4">
            No market data available. Subscribe to symbols in Market Data page.
          </div>
        ) : (
          <div className="space-y-2 max-h-64 overflow-y-auto">
            {ticks.map((tick, index) => (
              <div key={index} className="flex justify-between items-center p-2 border rounded hover:bg-muted/50">
                <div className="flex items-center gap-2">
                  <span className="font-medium">{tick.symbol}</span>
                  <Badge variant="outline" className="text-xs">
                    {tick.type}
                  </Badge>
                </div>
                <div className="text-right">
                  {tick.type === "TRADE" && (
                    <>
                      <div className="font-medium">${tick.trade_price?.toFixed(2)}</div>
                      <div className="text-xs text-muted-foreground">{tick.trade_size} shares</div>
                    </>
                  )}
                  {tick.type === "QUOTE" && (
                    <>
                      <div className="text-xs">
                        Bid: ${tick.bid_price?.toFixed(2)} x {tick.bid_size}
                      </div>
                      <div className="text-xs">
                        Ask: ${tick.ask_price?.toFixed(2)} x {tick.ask_size}
                      </div>
                    </>
                  )}
                </div>
                <div className="text-xs text-muted-foreground">{new Date(tick.timestamp).toLocaleTimeString()}</div>
              </div>
            ))}
          </div>
        )}
      </CardContent>
    </Card>
  )
}
