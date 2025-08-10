"use client"

import { useEffect, useState } from "react"
import { useParams } from "next/navigation"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"

interface Tick {
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

export default function SymbolPage() {
  const params = useParams<{ symbol: string }>()
  const symbol = decodeURIComponent(params.symbol)
  const [tick, setTick] = useState<Tick | null>(null)
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    let mounted = true
    const fetchIt = async () => {
      try {
        const res = await fetch(`http://localhost:18080/market/data/${encodeURIComponent(symbol)}`)
        if (!res.ok) return
        const data = await res.json()
        if (mounted) setTick(data)
      } finally {
        if (mounted) setLoading(false)
      }
    }
    fetchIt()
    const id = setInterval(fetchIt, 5000)
    return () => {
      mounted = false
      clearInterval(id)
    }
  }, [symbol])

  return (
    <div className="space-y-6">
      <Card>
        <CardHeader>
          <CardTitle>{symbol} Snapshot</CardTitle>
        </CardHeader>
        <CardContent>
          {loading ? (
            <div>Loading...</div>
          ) : !tick ? (
            <div>No data.</div>
          ) : (
            <div className="grid grid-cols-2 gap-4 text-sm">
              <div className="text-gray-500">Type</div>
              <div className="font-medium">{tick.type}</div>
              {tick.trade_price !== undefined && (
                <>
                  <div className="text-gray-500">Trade Price</div>
                  <div className="font-mono">${tick.trade_price?.toFixed(4)}</div>
                  <div className="text-gray-500">Trade Size</div>
                  <div className="font-mono">{tick.trade_size?.toLocaleString()}</div>
                </>
              )}
              {tick.bid_price !== undefined && (
                <>
                  <div className="text-gray-500">Bid</div>
                  <div className="font-mono">${tick.bid_price?.toFixed(4)} x {tick.bid_size}</div>
                  <div className="text-gray-500">Ask</div>
                  <div className="font-mono">${tick.ask_price?.toFixed(4)} x {tick.ask_size}</div>
                </>
              )}
              <div className="text-gray-500">Time</div>
              <div className="font-mono">{new Date(tick.timestamp).toLocaleString()}</div>
            </div>
          )}
        </CardContent>
      </Card>
    </div>
  )
}



